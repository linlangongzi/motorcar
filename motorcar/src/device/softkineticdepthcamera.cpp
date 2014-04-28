#include "softkineticdepthcamera.h"
#include "../scenegraph/output/viewpoint.h"
#include "../scenegraph/output/display/display.h"

using namespace motorcar;
using namespace DepthSense;
using namespace std;

Context g_context;
DepthNode g_dnode;
ColorNode g_cnode;
AudioNode g_anode;

uint32_t g_aFrames = 0;
uint32_t g_cFrames = 0;
uint32_t g_dFrames = 0;

bool g_bDeviceFound = false;

ProjectionHelper* g_pProjHelper = NULL;
StereoCameraParameters g_scp;


float *g_vertexData = 0;
float *g_confidenceData = 0;
float *g_uvData = 0;
int g_depthMapSize = 0;
bool g_depthUpdated;

ColorNode::NewSampleReceivedData g_colorData;


/*----------------------------------------------------------------------------*/
// New audio sample event handler
void onNewAudioSample(AudioNode node, AudioNode::NewSampleReceivedData data)
{
    //printf("A#%u: %d\n",g_aFrames,data.audioData.size());
    g_aFrames++;
}

/*----------------------------------------------------------------------------*/
// New color sample event handler
void onNewColorSample(ColorNode node, ColorNode::NewSampleReceivedData data)
{
    //printf("C#%u: %d\n",g_cFrames,data.colorMap.size());
    g_cFrames++;
    g_colorData = data;
}

/*----------------------------------------------------------------------------*/
// New depth sample event handler
void onNewDepthSample(DepthNode node, DepthNode::NewSampleReceivedData data)
{
    //std::cout << data.verticesFloatingPoint.size() << std::endl;

    // Project some 3D points in the Color Frame
    if (!g_pProjHelper)
    {
        g_pProjHelper = new ProjectionHelper (data.stereoCameraParameters);
        g_scp = data.stereoCameraParameters;
    }
    else if (g_scp != data.stereoCameraParameters)
    {
        g_pProjHelper->setStereoCameraParameters(data.stereoCameraParameters);
        g_scp = data.stereoCameraParameters;
    }

    if(data.verticesFloatingPoint.size() != g_depthMapSize){
        g_depthMapSize = data.verticesFloatingPoint.size();
        float * temp_ptr = g_vertexData;
        g_vertexData = new float[3 * g_depthMapSize];
        delete temp_ptr;
        temp_ptr = g_confidenceData;
        g_confidenceData = new float[g_depthMapSize];
        delete temp_ptr;
        temp_ptr = g_uvData;
        g_uvData = new float[2 * g_depthMapSize];
        delete temp_ptr;
    }

    for(int i=0; i < data.verticesFloatingPoint.size(); i++){
        DepthSense::FPVertex v = data.verticesFloatingPoint[i];
        //Geometry::printVector(glm::vec3(v.x, v.y, v.z));
        g_vertexData[i * 3 + 0] = v.x;
        g_vertexData[i * 3 + 1] = v.y;
        g_vertexData[i * 3 + 2] = v.z;
        g_confidenceData[i] = data.confidenceMap[i];
        DepthSense::UV t = data.uvMap[i];
        g_uvData[i *2 + 0] = t.u;
        g_uvData[i *2 + 1] = t.v;
        //std::cout << data.uvMap.size() << std::endl;
    }

    g_depthUpdated = true;

    g_dFrames++;


}

/*----------------------------------------------------------------------------*/
void configureAudioNode()
{
    g_anode.newSampleReceivedEvent().connect(&onNewAudioSample);

    AudioNode::Configuration config = g_anode.getConfiguration();
    config.sampleRate = 44100;

    try
    {
        g_context.requestControl(g_anode,0);

        g_anode.setConfiguration(config);

        g_anode.setInputMixerLevel(0.5f);
    }
    catch (ArgumentException& e)
    {
        printf("Argument Exception: %s\n",e.what());
    }
    catch (UnauthorizedAccessException& e)
    {
        printf("Unauthorized Access Exception: %s\n",e.what());
    }
    catch (ConfigurationException& e)
    {
        printf("Configuration Exception: %s\n",e.what());
    }
    catch (StreamingException& e)
    {
        printf("Streaming Exception: %s\n",e.what());
    }
    catch (TimeoutException&)
    {
        printf("TimeoutException\n");
    }
}

/*----------------------------------------------------------------------------*/
void configureDepthNode()
{
    g_dnode.newSampleReceivedEvent().connect(&onNewDepthSample);

    DepthNode::Configuration config = g_dnode.getConfiguration();
    config.frameFormat = FRAME_FORMAT_QVGA;
    config.framerate = 25;
    config.mode = DepthNode::CAMERA_MODE_CLOSE_MODE;
    config.saturation = true;

    g_dnode.setEnableVertices(true);
    g_dnode.setEnableVerticesFloatingPoint(true);
    g_dnode.setEnableConfidenceMap(true);
    g_dnode.setEnableUvMap(true);

    try
    {
        g_context.requestControl(g_dnode,0);

        g_dnode.setConfiguration(config);
    }
    catch (ArgumentException& e)
    {
        printf("Argument Exception: %s\n",e.what());
    }
    catch (UnauthorizedAccessException& e)
    {
        printf("Unauthorized Access Exception: %s\n",e.what());
    }
    catch (IOException& e)
    {
        printf("IO Exception: %s\n",e.what());
    }
    catch (InvalidOperationException& e)
    {
        printf("Invalid Operation Exception: %s\n",e.what());
    }
    catch (ConfigurationException& e)
    {
        printf("Configuration Exception: %s\n",e.what());
    }
    catch (StreamingException& e)
    {
        printf("Streaming Exception: %s\n",e.what());
    }
    catch (TimeoutException&)
    {
        printf("TimeoutException\n");
    }

}

/*----------------------------------------------------------------------------*/
void configureColorNode()
{
    // connect new color sample handler
    g_cnode.newSampleReceivedEvent().connect(&onNewColorSample);

    ColorNode::Configuration config = g_cnode.getConfiguration();
    config.frameFormat = FRAME_FORMAT_VGA;
    config.compression = COMPRESSION_TYPE_MJPEG;
    config.powerLineFrequency = POWER_LINE_FREQUENCY_50HZ;
    config.framerate = 25;

    g_cnode.setEnableColorMap(true);

    try
    {
        g_context.requestControl(g_cnode,0);

        g_cnode.setConfiguration(config);
    }
    catch (ArgumentException& e)
    {
        printf("Argument Exception: %s\n",e.what());
    }
    catch (UnauthorizedAccessException& e)
    {
        printf("Unauthorized Access Exception: %s\n",e.what());
    }
    catch (IOException& e)
    {
        printf("IO Exception: %s\n",e.what());
    }
    catch (InvalidOperationException& e)
    {
        printf("Invalid Operation Exception: %s\n",e.what());
    }
    catch (ConfigurationException& e)
    {
        printf("Configuration Exception: %s\n",e.what());
    }
    catch (StreamingException& e)
    {
        printf("Streaming Exception: %s\n",e.what());
    }
    catch (TimeoutException&)
    {
        printf("TimeoutException\n");
    }
}

/*----------------------------------------------------------------------------*/
void configureNode(Node node)
{
    if ((node.is<DepthNode>())&&(!g_dnode.isSet()))
    {
        g_dnode = node.as<DepthNode>();
        configureDepthNode();
        g_context.registerNode(node);
    }

    if ((node.is<ColorNode>())&&(!g_cnode.isSet()))
    {
        g_cnode = node.as<ColorNode>();
        configureColorNode();
        g_context.registerNode(node);
    }

    if ((node.is<AudioNode>())&&(!g_anode.isSet()))
    {
        g_anode = node.as<AudioNode>();
        configureAudioNode();
        g_context.registerNode(node);
    }
}

/*----------------------------------------------------------------------------*/
void onNodeConnected(Device device, Device::NodeAddedData data)
{
    configureNode(data.node);
}

/*----------------------------------------------------------------------------*/
void onNodeDisconnected(Device device, Device::NodeRemovedData data)
{
    if (data.node.is<AudioNode>() && (data.node.as<AudioNode>() == g_anode))
        g_anode.unset();
    if (data.node.is<ColorNode>() && (data.node.as<ColorNode>() == g_cnode))
        g_cnode.unset();
    if (data.node.is<DepthNode>() && (data.node.as<DepthNode>() == g_dnode))
        g_dnode.unset();
    printf("Node disconnected\n");
}

/*----------------------------------------------------------------------------*/
void onDeviceConnected(Context context, Context::DeviceAddedData data)
{
    if (!g_bDeviceFound)
    {
        data.device.nodeAddedEvent().connect(&onNodeConnected);
        data.device.nodeRemovedEvent().connect(&onNodeDisconnected);
        g_bDeviceFound = true;
    }
}

/*----------------------------------------------------------------------------*/
void onDeviceDisconnected(Context context, Context::DeviceRemovedData data)
{
    g_bDeviceFound = false;
    printf("Device disconnected\n");
}


void cameraEventLoop() {

        g_context.startNodes();

        g_context.run();

        g_context.stopNodes();

        std::cout << "unregistering depth camera" <<std::endl;

        if (g_cnode.isSet()) g_context.unregisterNode(g_cnode);
        if (g_dnode.isSet()) g_context.unregisterNode(g_dnode);
        if (g_anode.isSet()) g_context.unregisterNode(g_anode);

        if (g_pProjHelper)
            delete g_pProjHelper;
}


SoftKineticDepthCamera::SoftKineticDepthCamera(SceneGraphNode *parent, const glm::mat4 &transform)
    :Drawable(parent, transform)
    ,m_pointCloudShader(new motorcar::OpenGLShader(std::string("../motorcar/src/shaders/softkineticdepthcam.vert"), std::string("../motorcar/src/shaders/softkineticdepthcam.frag")))
{


    g_context = Context::create("localhost");

    g_context.deviceAddedEvent().connect(&onDeviceConnected);
    g_context.deviceRemovedEvent().connect(&onDeviceDisconnected);

    // Get the list of currently connected devices
    vector<Device> da = g_context.getDevices();

    // We are only interested in the first device
    if (da.size() >= 1)
    {
        g_bDeviceFound = true;

        da[0].nodeAddedEvent().connect(&onNodeConnected);
        da[0].nodeRemovedEvent().connect(&onNodeDisconnected);

        vector<Node> na = da[0].getNodes();

        printf("Found %u nodes\n",na.size());

        for (int n = 0; n < (int)na.size();n++)
            configureNode(na[n]);
    }

    m_cameraThread = std::thread(cameraEventLoop);

    h_aPosition =  glGetAttribLocation(m_pointCloudShader->handle(), "aPosition");
    h_aConfidence =  glGetAttribLocation(m_pointCloudShader->handle(), "aConfidence");
    h_aTexCoord =  glGetAttribLocation(m_pointCloudShader->handle(), "aTexCoord");
    h_uMVPMatrix  = glGetUniformLocation(m_pointCloudShader->handle(), "uMVPMatrix");

    if(h_aPosition < 0 || h_aConfidence < 0 || h_aTexCoord < 0 || h_uMVPMatrix < 0){
       std::cout << "problem with point cloud shader handles: " << h_aPosition << ", " << h_aConfidence << ", "<< h_aTexCoord << ", "<< h_uMVPMatrix << std::endl;
    }

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    GLushort indices[(m_colorWidth - 1)][(m_colorHeight - 1)][6];
    for(int i = 0; i < m_colorWidth - 1; i++){
        for(int j=0; j< m_colorHeight - 1; j++){
            int tl = j * m_colorWidth + i;
            int tr = j * m_colorWidth + i + 1;
            int bl = (j + 1) * m_colorWidth + i;
            int br = (j + 1) * m_colorWidth + i + 1;

            indices[i][j][0] = tl;
            indices[i][j][1] = bl;
            indices[i][j][2] = tr;
            indices[i][j][3] = tr;
            indices[i][j][4] = bl;
            indices[i][j][5] = br;
        }
    }

    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

SoftKineticDepthCamera::~SoftKineticDepthCamera()
{
    std::cout << "stopping depth camera" <<std::endl;
    g_context.quit();
    m_cameraThread.join();

    std::cout << "depth camera stopped" <<std::endl;

    delete m_pointCloudShader;
}

void SoftKineticDepthCamera::draw(Scene *scene, Display *display)
{


    glPointSize( 2.0 );

    //std::cout << g_depthData.verticesFloatingPoint.size() <<std::endl;

    glUseProgram(m_pointCloudShader->handle());

    glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(h_aConfidence);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(h_aTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, g_vertexData);
    glVertexAttribPointer(h_aConfidence, 1, GL_FLOAT, GL_FALSE, 0, g_confidenceData);
    glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, g_uvData);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_colorWidth, m_colorHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, g_colorData.colorMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    //Geometry::printVector(glm::vec3(g_vertexData[0], g_vertexData[1], g_vertexData[2]));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    for(ViewPoint *viewpoint : display->viewpoints()){
        //Geometry::printMatrix(viewpoint->viewMatrix());
        glUniformMatrix4fv(h_uMVPMatrix, 1, GL_FALSE, glm::value_ptr(viewpoint->projectionMatrix() * viewpoint->viewMatrix() *  this->worldTransform()));
        viewpoint->viewport()->set();
        //glDrawArrays(GL_POINTS, 0, g_depthMapSize);

        //glDrawElements(GL_TRIANGLES, (m_colorWidth - 1)*(m_colorHeight - 1)* 6, GL_UNSIGNED_INT, (GLvoid*)0);
        glDrawElements(GL_TRIANGLES, m_colorWidth*m_colorHeight, GL_UNSIGNED_SHORT, (GLvoid*)0);



    }


    glDisableVertexAttribArray(h_aPosition);
    glDisableVertexAttribArray(h_aConfidence);
    glDisableVertexAttribArray(h_aTexCoord);

    glUseProgram(0);

}


