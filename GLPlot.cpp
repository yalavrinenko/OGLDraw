//
// Created by cheshire on 11.06.17.
//

#include "GLPlot.hpp"
#include <unordered_map>
#include <thread>
#include "Logger.hpp"

std::unordered_map<std::thread::id, OGLDraw::Window*> g_Windows;

int OGLDraw::Window::window_count=0;

void OGLDraw::mainOpenGLDisplayFunction(){
    //Logger::Info(std::this_thread::get_id(), g_Windows[std::this_thread::get_id()]->m_windowName);
    try {
        g_Windows[std::this_thread::get_id()]->display_function();
    }
    catch (std::exception){
    };
}

void OGLDraw::mainOpenGLReshapeFunction(int w, int h){
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); //Switch to setting the camera perspective
    glLoadIdentity(); //Reset the camera
}

void OGLDraw::Window::init(int argc,char ** argv, DrawFunction windowProps,
                           DrawFunction addition, DrawFunction drawFunc) {
    m_userDisplayFunction = drawFunc;

    if (window_count == 0)
        glutInit(&argc, argv);

    ++window_count;

    m_idleThread = std::thread([&](){
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowSize(800, 800);

        if (windowProps!= nullptr)
            windowProps();

        m_glutWinId=glutCreateWindow(m_windowName.c_str()); //Create a window

        glEnable(GL_DEPTH_TEST); //Make sure 3D drawing works when one object is in front of another

        glEnable(GL_POINT_SMOOTH);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        glEnable(GL_COLOR_MATERIAL);

        // Create light components.
        GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
        GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
        GLfloat position[] = { 5.0f, 5.0f, 5.0f, 1.0f };

        // Assign created components to GL_LIGHT0.
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
        glLightfv(GL_LIGHT0, GL_POSITION, position);

        if (addition)
            addition();

        glutDisplayFunc(mainOpenGLDisplayFunction);
        glutReshapeFunc(mainOpenGLReshapeFunction);

        this->isIdle = true;
        this->m_idleFunc();
    });
}

void OGLDraw::Window::m_idleFunc() {
    g_Windows[std::this_thread::get_id()]=this;

    std::unique_lock<std::mutex> lock(this->m_idleMutex);
    while (isIdle){
        while (!isFrameReady && isIdle)
            this->m_idleCondition.wait(lock);

        if (isIdle) {
            mainOpenGLDisplayFunction();
            isFrameReady = false;

            std::unique_lock<std::mutex> isReady(m_notifyMutex);
            isNotifyReady = true;
            m_notifyCondition.notify_all();
        }
    }
    Logger::Warning("Stop IDLE.");
    g_Windows.erase(std::this_thread::get_id());
}

void OGLDraw::Window::draw() {
    std::unique_lock<std::mutex> isReady(m_notifyMutex);
    while (!isNotifyReady)
        m_notifyCondition.wait(isReady);

    std::unique_lock<std::mutex> lock(this->m_idleMutex);
    isFrameReady = true;
    isNotifyReady = false;
    this->m_idleCondition.notify_all();
}

void OGLDraw::Window::display_function() {
    //Logger::Info("Call display function from", m_windowName);
    glutSetWindow(m_glutWinId);

    if (m_userDisplayFunction){
        m_userDisplayFunction();
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); //Switch to the drawing perspective
    glLoadIdentity();

    for (auto func: m_functionList)
        func();

    for (auto object: m_objectList)
        object->draw();

    glFlush();
    glutSwapBuffers();
}
