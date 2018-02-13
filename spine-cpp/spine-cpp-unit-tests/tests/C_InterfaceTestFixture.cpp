#include "C_InterfaceTestFixture.h" 
#include "SpineEventMonitor.h" 

#include <spine/SkeletonJson.h>
#include <spine/SkeletonData.h>
#include <spine/Atlas.h>
#include <spine/AnimationStateData.h>
#include <spine/Skeleton.h>
#include <spine/AnimationState.h>
#include <spine/Animation.h>

#include <vector>
#include <spine/Extension.h>
#include <spine/TextureLoader.h>
#include <spine/Vector.h>

#include <spine/CurveTimeline.h>
#include <spine/VertexAttachment.h>
#include <spine/Json.h>

#include <spine/AttachmentLoader.h>
#include <spine/AtlasAttachmentLoader.h>
#include <spine/LinkedMesh.h>

#include <new>

#include "KMemory.h" // last include

#define SPINEBOY_JSON "testdata/spineboy/spineboy-ess.json"
#define SPINEBOY_ATLAS "testdata/spineboy/spineboy.atlas"

#define RAPTOR_JSON "testdata/raptor/raptor-pro.json"
#define RAPTOR_ATLAS "testdata/raptor/raptor.atlas"

#define GOBLINS_JSON "testdata/goblins/goblins-pro.json"
#define GOBLINS_ATLAS "testdata/goblins/goblins.atlas"

#define MAX_RUN_TIME 6000 // equal to about 100 seconds of execution

void C_InterfaceTestFixture::setUp()
{
}

void C_InterfaceTestFixture::tearDown()
{
}

static Spine::SkeletonData* readSkeletonJsonData(const char* filename, Atlas* atlas)
{
    using namespace Spine;

    Vector<Atlas*> atlasArray;
    atlasArray.push_back(atlas);
    
    SkeletonJson* skeletonJson = NEW(SkeletonJson);
    new (skeletonJson) SkeletonJson(atlasArray);
	ASSERT(skeletonJson != 0);

	SkeletonData* skeletonData = skeletonJson->readSkeletonDataFile(filename);
	ASSERT(skeletonData != 0);

    DESTROY(SkeletonJson, skeletonJson);
    
	return skeletonData;
}

typedef std::vector<std::string> AnimList;

static size_t enumerateAnimations(AnimList& outList, SkeletonData* skeletonData)
{
	if (skeletonData)
    {
		for (int n = 0; n < skeletonData->getAnimations().size(); n++)
        {
            outList.push_back(skeletonData->getAnimations()[n]->getName());
        }
	}

	return outList.size();
}

class MyTextureLoader : public TextureLoader
{
    virtual void load(AtlasPage& page, std::string path)
    {
        page.rendererObject = NULL;
        page.width = 2048;
        page.height = 2048;
    }
    
    virtual void unload(void* texture)
    {
        // TODO
    }
};

static void testRunner(const char* jsonName, const char* atlasName)
{    
	///////////////////////////////////////////////////////////////////////////
	// Global Animation Information
    MyTextureLoader myTextureLoader;
    Atlas* atlas = NEW(Atlas);
    new (atlas) Atlas(atlasName, myTextureLoader);
	ASSERT(atlas != 0);

	SkeletonData* skeletonData = readSkeletonJsonData(jsonName, atlas);
	ASSERT(skeletonData != 0);

    AnimationStateData* stateData = NEW(AnimationStateData);
    new (stateData) AnimationStateData(skeletonData);
	ASSERT(stateData != 0);
	stateData->setDefaultMix(0.2f); // force mixing

	///////////////////////////////////////////////////////////////////////////
	// Animation Instance 
    Skeleton* skeleton = NEW(Skeleton);
    new (skeleton) Skeleton(skeletonData);
	ASSERT(skeleton != 0);

    AnimationState* state = NEW(AnimationState);
    new (state) AnimationState(stateData);
	ASSERT(state != 0);

	///////////////////////////////////////////////////////////////////////////
	// Run animation
	skeleton->setToSetupPose();
    SpineEventMonitor eventMonitor(state);

	AnimList anims; // Let's chain all the animations together as a test
	size_t count = enumerateAnimations(anims, skeletonData);
	if (count > 0)
    {
        state->setAnimation(0, anims[0].c_str(), false);
    }
    
	for (size_t i = 1; i < count; ++i)
    {
        state->addAnimation(0, anims[i].c_str(), false, 0.0f);
	}

	// Run Loop
	for (int i = 0; i < MAX_RUN_TIME && eventMonitor.isAnimationPlaying(); ++i)
    {
		const float timeSlice = 1.0f / 60.0f;
		skeleton->update(timeSlice);
		state->update(timeSlice);
		state->apply(*skeleton);
	}
	
	///////////////////////////////////////////////////////////////////////////
	// Dispose Instance
    DESTROY(Skeleton, skeleton);
    DESTROY(AnimationState, state);

	///////////////////////////////////////////////////////////////////////////
	// Dispose Global
    DESTROY(AnimationStateData, stateData);
    DESTROY(SkeletonData, skeletonData);
    DESTROY(Atlas, atlas);
}

void C_InterfaceTestFixture::spineboyTestCase()
{
	testRunner(SPINEBOY_JSON, SPINEBOY_ATLAS);
}

void C_InterfaceTestFixture::raptorTestCase()
{
	testRunner(RAPTOR_JSON, RAPTOR_ATLAS);
}

void C_InterfaceTestFixture::goblinsTestCase()
{
	testRunner(GOBLINS_JSON, GOBLINS_ATLAS);
}