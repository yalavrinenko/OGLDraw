//
// Created by cheshire on 11.06.17.
//

#ifndef GLPLOT_HPP
#define GLPLOT_HPP

#include <string>
#include <list>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

namespace OGLDraw {

    void mainOpenGLDisplayFunction();
    void mainOpenGLReshapeFunction(int w, int h);

    typedef std::function<void(void)> DrawFunction;

    class Object {
    private:

    public:
        virtual void draw() = 0;
    };

    class Window {
    public:
        Window(std::string windowName = "OGLDraw window") : m_windowName(windowName) {}

        void init(int argc, char ** argv,DrawFunction windowProps = nullptr, DrawFunction addition = nullptr,
                  DrawFunction drawFunc= nullptr);

        void draw();

        void addObject(std::shared_ptr<Object> object){
            m_objectList.push_back(object);
        }

        void addFunction(DrawFunction function){
            m_functionList.push_back(function);
        }

        template <class ... OArgs>
                void  addObject(std::shared_ptr<Object> object, OArgs... args){
            addObject(object);
            addObject(args...);
        }

        template <class ... FArgs>
                void addFunction(DrawFunction function, FArgs... args){
            addFunction(function);
            addFunction(args...);
        }

        void close(){
            if (isIdle) {
                {
                    std::unique_lock<std::mutex> lock(this->m_idleMutex);
                    isIdle = false;
                    isFrameReady = true;
                }
                m_idleCondition.notify_all();
                m_idleThread.join();
            }
        }

        ~Window(){
            close();
        }

    private:
        int m_glutWinId;
        static int window_count;

        std::string m_windowName;
        std::list<std::shared_ptr<Object>> m_objectList;
        std::list<DrawFunction> m_functionList;

        DrawFunction m_userDisplayFunction;

        std::thread m_idleThread;
        bool isFrameReady = false;
        bool isIdle = false;

        std::condition_variable m_idleCondition;
        std::mutex m_idleMutex;

        std::condition_variable m_notifyCondition;
        std::mutex m_notifyMutex;
        bool isNotifyReady = true;

        friend void mainOpenGLDisplayFunction();

        void display_function();

        void m_idleFunc();

        Window(Window const &w);
        Window(Window &&w);
        Window operator =(Window const &w);
    };
};

#endif //GLPLOT_HPP