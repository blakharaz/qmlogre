#pragma once

#include <OGRE/OgreTexture.h>

#include <QImage>
#include <QtQuick/QQuickRenderControl>
#include <QtQuick/QQuickWindow>
#include <QtQml/QQmlComponent>
#include <QtQml/QQmlEngine>

class UiRenderer
{
public:
    UiRenderer(const Ogre::Texture &texture);

    void setSource(const std::string &url);
    void render();
    uchar* getPixels();
    const uchar* getPixels() const;
    void saveToFile(const std::string &path);

private:
    std::atomic<bool> m_initialized;
    std::unique_ptr<QImage> m_pixels;
    std::unique_ptr<QQuickWindow> m_quickWindow;
    std::unique_ptr<QQuickRenderControl> m_renderControl;
    std::unique_ptr<QQmlEngine> m_qmlEngine;
    std::unique_ptr<QQmlComponent> m_qmlRoot;
};
