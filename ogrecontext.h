#pragma once

#include <OGRE/Overlay/OgreOverlayManager.h>
#include <OGRE/Bites/OgreApplicationContextQt.h>
#include <OGRE/Bites/OgreCameraMan.h>

#include "uirenderer.h"

class OgreContext : public OgreBites::ApplicationContextQt
{
public:
    void createRoot() override;
    void locateResources() override;
    void setup() override;

    bool frameStarted(const Ogre::FrameEvent& evt) override;

    void setUiSource(const std::string &url);

private:
    void createUiTexture();
    void setupScene();

    size_t mTextureSize;

    std::unique_ptr<UiRenderer> mUiRenderer;
    std::shared_ptr<Ogre::Material> mOverlayMaterial;
    std::shared_ptr<Ogre::Texture> mUiTexture;
    std::unique_ptr<OgreBites::CameraMan> mCameraMan;
};
