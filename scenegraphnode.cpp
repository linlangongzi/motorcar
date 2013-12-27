#include "scenegraphnode.h"

SceneGraphNode::SceneGraphNode(QObject *parent) :
    QObject(parent)
{ 
    if(dynamic_cast<SceneGraphNode*>(parent) != NULL){
        SceneGraphNode *parentNode = dynamic_cast<SceneGraphNode*>(parent);
        setParentNode(parentNode);
        parentNode->addChildNode(this);
    }
    
}

SceneGraphNode::~SceneGraphNode(){
    if (this->parentNode() != NULL)
        this->parentNode()->removeChildNode(this);

    foreach (SceneGraphNode *child, m_childNodes) {
        delete child;
    }
}

bool SceneGraphNode::animate(const float deltaTime)
{
    return true;
}

bool SceneGraphNode::draw() const
{
    return true;
}


void SceneGraphNode::traverseChildren(float deltaTime){
    foreach (SceneGraphNode *child, m_childNodes) {
        if (child != NULL){
            child->traverse(deltaTime);
        }
    }

}

bool SceneGraphNode::traverse(float deltaTime){

    if(!this->animate(deltaTime) || !this->draw()){
        return false;
    }
    this->traverseChildren(deltaTime);
    return true;
}




void SceneGraphNode::setParentNode(SceneGraphNode *parent)
{
    if (this->parentNode() != NULL){
        this->parentNode()->removeChildNode(this);
    }
    this->m_parentNode = parent;
}

void SceneGraphNode::addChildNode(SceneGraphNode *child)
{
    child->setParent(this);
    this->m_childNodes.append(child);

}

void SceneGraphNode::removeChildNode(SceneGraphNode *node)
{
    this->m_childNodes.removeOne(node);
}

bool SceneGraphNode::existsInSubtree(SceneGraphNode *node)
{
    if(node == this) return true;
    foreach (SceneGraphNode *child, m_childNodes) {
        if (child != NULL && child->existsInSubtree(node)){
             return true;

        }
    }
    return false;

}

MotorcarSurfaceNode *SceneGraphNode::getSurfaceNode(const QWaylandSurface *surface)
{
    foreach (SceneGraphNode *child, m_childNodes) {
        if (child != NULL){
            MotorcarSurfaceNode *node = child->getSurfaceNode(surface);
            if(node) return node;
        }
    }
    return NULL;

}

glm::mat4 SceneGraphNode::worldTransform()
{
    if(this->m_parentNode != NULL){
        return this->parentNode()->worldTransform() * this->transform();
    }else{
        return this->transform();
    }
}


SceneGraphNode *SceneGraphNode::parentNode() const
{
    return this->m_parentNode;
}


glm::mat4 SceneGraphNode::transform() const
{
    return m_transform;
}

void SceneGraphNode::setTransform(const glm::mat4 &value)
{
    m_transform = value;
}

