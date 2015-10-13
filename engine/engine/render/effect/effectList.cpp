#include "mcv_platform.h"

#include "effect.h"
#include "tools.h"
#include "blur.h"
#include "Smudge.h"
#include "ssao.h"

namespace render {

/*
    Creating an effect (similar to the component system):

    1- create the effect class
        - inherits Effect and implements its virtual methods
        - has constructor with signature (std::string name, int xRes, int yRes)
        - typically, it will create at least one RenderedTexture using these
        - see BasicEffect for an example
        - If you need auxiliary textures, it's a good idea to use Fetch objects
        - Then you can add the fetch as a subtag in the xml
        - see Mix for an example
    2- register the effect class
        - add it in EffectLibrary::init() with the line FX_REGISTER(Class);
        - add it to tearDown if necessary;
    3- use the shader
        - Add it to the corresponding pipeline as an xml tag.
        - The tag's name is the class name
        - The pipeline will call loadFromProperties() for the corresponding tag and all tags inside it
        - After reading each effect, it will call its init() method
        - Name the stage using a "name" attribute if you plan to refer to it from another stage
        - You can fetch the output of a stage by name, using PostProcessPipeline::getStage()
        - The pipeline can register a series of resources to be used by the effects (such as the depth buffer).
        - This is hardcoded in the engine and can be referred by a name using PostProcessPipeline::getResource()
        - The entry texture is registered as "INPUT"
        
        Using tool stages like Fetch or Mix allow pipelines to become high level programs. The Fetch tool
        doesn't do any pass, just fetches a texture to use as input to another stage. You can use this to
        work on auxiliary/mixing textures, then fetch the previous stage again.
*/

void EffectLibrary::init()
{
    FX_REGISTER(BasicEffect);
#ifdef _DEBUG
    FX_REGISTER(DebugFX);
#endif
    FX_REGISTER(Fetch);
    FX_REGISTER(Mix);
    FX_REGISTER(Blur);
    FX_REGISTER(MotionBlur);
    FX_REGISTER(Smudge);
    FX_REGISTER(SSAO); 
    
    Mix::initType();
    Blur::initType();
    MotionBlur::initType();
    Smudge::initType();
    SSAO::initType();
}

void EffectLibrary::tearDown()
{
    Mix::tearDown();
    Blur::tearDown();
    MotionBlur::tearDown();
    Smudge::tearDown();
    SSAO::tearDown();
}

}