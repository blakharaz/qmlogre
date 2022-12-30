#include "ogrecontext.h"

#include <OGRE/OgreConfigFile.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgreTextureManager.h>
#include <OGRE/OgreViewport.h>

#include <OGRE/Overlay/OgreOverlay.h>
#include <OGRE/Overlay/OgreOverlayContainer.h>
#include <OGRE/Overlay/OgreOverlayManager.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>

void OgreContext::createRoot()
{
    Ogre::String pluginsPath;

    mRoot = new Ogre::Root("plugins.cfg", "ogre.cfg", "./ogre.log");
    mStaticPluginLoader.load();
    mOverlaySystem = new Ogre::OverlaySystem();
}

void OgreContext::locateResources()
{
    auto &rgm = Ogre::ResourceGroupManager::getSingleton();
    // load resource paths from config file
    Ogre::ConfigFile cf;
    Ogre::String resourcesPath = mFSLayer->getConfigFilePath("resources.cfg");

    if (Ogre::FileSystemLayer::fileExists(resourcesPath))
    {
        Ogre::LogManager::getSingleton().logMessage("Parsing '" + resourcesPath + "'");
        cf.load(resourcesPath);
    }
    else
    {
        rgm.addResourceLocation(getDefaultMediaDir(), "FileSystem", Ogre::RGN_DEFAULT);
    }

    Ogre::String sec, type, arch;
    // go through all specified resource groups
    Ogre::ConfigFile::SettingsBySection_::const_iterator seci;
    for (seci = cf.getSettingsBySection().begin(); seci != cf.getSettingsBySection().end(); ++seci)
    {
        sec = seci->first;
        const Ogre::ConfigFile::SettingsMultiMap &settings = seci->second;
        Ogre::ConfigFile::SettingsMultiMap::const_iterator i;

        // go through all resource paths
        for (i = settings.begin(); i != settings.end(); i++)
        {
            type = i->first;
            arch = i->second;

            Ogre::StringUtil::trim(arch);
            if (arch.empty() || arch[0] == '.')
            {
                // resolve relative path with regards to configfile
                Ogre::String baseDir, filename;
                Ogre::StringUtil::splitFilename(resourcesPath, filename, baseDir);
                arch = baseDir + arch;
            }

            arch = Ogre::FileSystemLayer::resolveBundlePath(arch);

#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
            if ((type == "Zip" || type == "FileSystem") && !Ogre::FileSystemLayer::fileExists(arch))
            {
                Ogre::LogManager::getSingleton().logWarning("resource location '" + arch +
                                                            "' does not exist - skipping");
                continue;
            }
#endif
            rgm.addResourceLocation(arch, type, sec);
        }
    }

    if (rgm.getResourceLocationList(Ogre::RGN_INTERNAL).empty())
    {
        const auto &mediaDir = getDefaultMediaDir();
        // add default locations
        rgm.addResourceLocation(mediaDir + "/Main", "FileSystem", Ogre::RGN_INTERNAL);
#ifdef OGRE_BUILD_COMPONENT_TERRAIN
        rgm.addResourceLocation(mediaDir + "/Terrain", "FileSystem", Ogre::RGN_INTERNAL);
#endif
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        rgm.addResourceLocation(mediaDir + "/RTShaderLib/GLSL", "FileSystem", Ogre::RGN_INTERNAL);
        rgm.addResourceLocation(mediaDir + "/RTShaderLib/HLSL_Cg", "FileSystem", Ogre::RGN_INTERNAL);
#endif
    }
}

void OgreContext::setup()
{
    mRoot->initialise(false);
    createWindow(mAppName);

    locateResources();
    initialiseRTShaderSystem();
    loadResources();

    createUiTexture();

    setupScene();

    // adds context as listener to process context-level (above the sample level) events
    mRoot->addFrameListener(this);
}

void OgreContext::setupScene()
{
    // get a pointer to the already created root
    Ogre::Root *root = getRoot();
    Ogre::SceneManager *scnMgr = root->createSceneManager();
    scnMgr->addRenderQueueListener(mOverlaySystem);

    scnMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator *shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    // without light we would just get a black screen
    Ogre::Light *light = scnMgr->createLight("MainLight");
    Ogre::SceneNode *lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    lightNode->attachObject(light);
    lightNode->setPosition(20, 80, 50);

    // create the camera
    Ogre::SceneNode *camNode = scnMgr->getRootSceneNode()->createChildSceneNode();

    Ogre::Camera *cam = scnMgr->createCamera("myCam");
    cam->setNearClipDistance(5); // specific to this sample
    cam->setAutoAspectRatio(true);
    camNode->attachObject(cam);
    camNode->setPosition(0, 47, 222);

    // and tell it to render into the main window
    getRenderWindow()->addViewport(cam);

    //    mCameraMan = std::make_unique<OgreBites::CameraMan>(camNode);
    //    mCameraMan->setStyle(OgreBites::CS_ORBIT);

    Ogre::Entity *ogreEntity = scnMgr->createEntity("ogrehead.mesh");
    Ogre::SceneNode *ogreNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    ogreNode->attachObject(ogreEntity);

    Ogre::Entity *ogreEntity2 = scnMgr->createEntity("ogrehead.mesh");
    Ogre::SceneNode *ogreNode2 = scnMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(84, 48, 0));
    ogreNode2->attachObject(ogreEntity2);
}

void OgreContext::createUiTexture()
{
    auto &overlay_manager = Ogre::OverlayManager::getSingleton();
    auto *overlay = overlay_manager.create("overlay");
    mOverlayMaterial = Ogre::MaterialManager::getSingleton().create(
        "overlay_OverlayMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    auto *overlayPanel =
        dynamic_cast<Ogre::OverlayContainer *>(overlay_manager.createOverlayElement("Panel", "overlay_Panel"));
    overlayPanel->setPosition(0.0, 0.0);
    overlayPanel->setDimensions(1.0, 1.0);
    overlayPanel->setMaterialName(mOverlayMaterial->getName());

    overlay->add2D(overlayPanel);

    uint texture_width = 512;
    uint texture_height = 512;

    mTextureSize = texture_width * texture_height * 4;

    mUiTexture = Ogre::TextureManager::getSingleton().createManual(
        "overlay_OverlayTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D,
        texture_width, texture_height, 0, Ogre::PF_A8R8G8B8, Ogre::TU_DEFAULT);

    mOverlayMaterial->setLightingEnabled(false);
    mOverlayMaterial->setDepthCheckEnabled(false);
    mOverlayMaterial->setDepthWriteEnabled(false);

    auto pass = mOverlayMaterial->getTechnique(0)->getPass(0);
    pass->setLightingEnabled(false);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    pass->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
    pass->setDepthCheckEnabled(false);
    pass->setDepthWriteEnabled(false);
    pass->setVertexColourTracking(Ogre::TVC_DIFFUSE);
    auto tu = pass->createTextureUnitState(mUiTexture->getName());
    tu->setTextureAddressingMode(Ogre::TAM_CLAMP);
    tu->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_NONE);

    mOverlayMaterial->touch();

    overlay->show();

    mUiRenderer = std::make_unique<UiRenderer>(*mUiTexture);
}

bool OgreContext::frameStarted(const Ogre::FrameEvent &evt)
{
    bool result = OgreBites::ApplicationContextQt::frameStarted(evt);

    mUiRenderer->render();

    auto pixels = mUiRenderer->getPixels();
    Ogre::PixelBox box(512, 512, 1, Ogre::PF_A8R8G8B8, pixels);

    mUiTexture->getBuffer()->blitFromMemory(box);

    return result;
}

void OgreContext::setUiSource(const std::string& url)
{
    mUiRenderer->setSource(url);
}
