#include "uirenderer.h"

#include <QtQuick/QQuickGraphicsDevice>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickRenderTarget>
#include <QtQuick/QQuickWindow>

#include <OGRE/OgreLogManager.h>

UiRenderer::UiRenderer(const Ogre::Texture& texture)
{
    m_renderControl = std::make_unique<QQuickRenderControl>();

    m_pixels = std::make_unique<QImage>(texture.getWidth(), texture.getHeight(), QImage::Format::Format_ARGB32);


    m_quickWindow = std::make_unique<QQuickWindow>(m_renderControl.get());
    m_quickWindow->setRenderTarget(QQuickRenderTarget::fromPaintDevice(m_pixels.get()));
    m_quickWindow->setColor(QColor::fromRgb(0, 0, 0, 0));

    // Create a QML engine.
    m_qmlEngine = std::make_unique<QQmlEngine>();
    if (!m_qmlEngine->incubationController())
    {
        m_qmlEngine->setIncubationController(m_quickWindow->incubationController());
    }

    QObject::connect(m_qmlEngine.get(), &QQmlEngine::warnings,
                     [&](const QList<QQmlError> &warnings)
    {
        for (const auto& warning : warnings)
        {
            Ogre::LogManager::getSingleton().logWarning(warning.description().toStdString());
        }
    });
}

void UiRenderer::render()
{
    m_renderControl->polishItems();

    m_renderControl->beginFrame();
    m_renderControl->sync();
    m_renderControl->render();
    m_renderControl->endFrame(); // Qt Quick's rendering commands are submitted to the device context here
}

uchar* UiRenderer::getPixels()
{
    return m_pixels->bits();
}

const uchar* UiRenderer::getPixels() const
{
    return m_pixels->constBits();
}

void UiRenderer::saveToFile(const std::string& path)
{
    m_pixels->save(QString::fromStdString(path));
}

void UiRenderer::setSource(const std::string& url)
{
    QQmlComponent component(m_qmlEngine.get(), QString::fromStdString(url));
    QQuickItem* root = static_cast<QQuickItem*>(component.create());

    root->setParentItem(m_quickWindow->contentItem());
}
