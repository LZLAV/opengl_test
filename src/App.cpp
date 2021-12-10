#include "App.h"

App::App()
{
    _view = new View(); 
    viewContext = (ViewContext*)malloc(sizeof(ViewContext));
}

App::~App()
{
    if(_view != nullptr)
    {
        delete _view;
    }
}

int App::init()
{
    viewContext->width = 544;
    viewContext->height = 960;
    viewContext->title = "opengl test";
    _view->init(viewContext);
    _view->createWin();
    _view->configegl(nullptr);
    _view->configGL();
    return 0;
}

int App::draw()
{   
    _view->draw();
    _view->swap();
    return 0;
}

int main (int argc,char **argv)
{   
    App app;
    app.init();
    app.draw();
    char ch;
    std::cin>>ch;
    return 0;
}
