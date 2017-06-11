//
// Created by cheshire on 11.06.17.
//

#include "GLPlot.hpp"
#include "Logger.hpp"
#include <chrono>
#include <X11/Xlib.h>
#include <cmath>

class Line: public OGLDraw::Object{
private:
    double dx=0.01;
    int point =0;
    std::list<double> m_points;
    std::function<double(double)> func;
    int point_limit = 1000;
    double scale_y=1;
    double maxy=1;
public:
    Line(std::function<double(double)> _func): func(_func){

    }

    void draw(){
        auto y=func(dx * (double)point);

        if (fabs(y) > maxy){
            maxy = fabs(y);
            scale_y = 1.0 / maxy;
        }

        m_points.push_back(y);
        ++point;
        if (m_points.size() > 1000)
            m_points.pop_front();

        glPushMatrix();
        glTranslated(-1.0, 0.0, 0.0);

        glColor3d(1,0,0);

        glPointSize(5);
        glBegin(GL_POINTS);{
            double ddx=2.0 / 1000;
            int idx = 0;
            for (auto y : m_points) {
                glVertex2d(ddx * idx, y*scale_y);
                ++idx;
            }
        }
        glEnd();

        glPopMatrix();
    }
};

int main(int argc,char ** argv){
    XInitThreads();

    OGLDraw::Window w2("Window2");//, w1("Window1"), w3;
    w2.init(argc, argv);

    OGLDraw::Window w1("Window1");

    w1.init(argc, argv, [](){glutInitWindowSize(400, 400);});

    auto axis = [](){
        glColor3d(0,1,0);
        glBegin(GL_LINES);
        {
            glVertex2d(-1.0, 0);
            glVertex2d(1.0, 0);

            glVertex2d(0, 1);
            glVertex2d(0, -1);
        }
        glEnd();
    };

    w2.addFunction(axis);
    w2.addObject(std::shared_ptr<OGLDraw::Object>(new Line([](double x){return exp(cos(x))*sin(x);})));

    w1.addFunction(axis);
    w1.addObject(std::shared_ptr<OGLDraw::Object>(new Line([](double x){return cos(x) + sin(x);})));

    for (int i=0; i<10000; ++i){
        w1.draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        w2.draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    int a;
    std::cin >> a;
    return 0;
}

