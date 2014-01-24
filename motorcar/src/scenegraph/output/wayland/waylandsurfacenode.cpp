#include "waylandsurfacenode.h"
using namespace motorcar;

WaylandSurfaceNode::WaylandSurfaceNode(WaylandSurface *surface, SceneGraphNode &parent, const glm::mat4 &transform)
    :WaylandDrawable(parent, transform)
{
    this->setSurface(surface);
}

WaylandSurface *WaylandSurfaceNode::surface() const
{
    return m_surface;
}

void WaylandSurfaceNode::setSurface(WaylandSurface *surface)
{
    m_surface = surface;
}

glm::mat4 WaylandSurfaceNode::surfaceTransform() const
{
    return m_surfaceTransform;
}

void WaylandSurfaceNode::computeSurfaceTransform(float ppcm)
{
    if(ppcm > 0){
        float ppm = ppcm * 100.f;
        glm::mat4 surfaceRotation = glm::rotate(glm::mat4(1), 180.f ,glm::vec3(0, 0, 1));
        glm::mat4 surfaceScale = glm::scale(glm::mat4(1), glm::vec3( m_surface->size().width() / ppm,  m_surface->size().height() / ppm, 1));
        glm::mat4 surfaceOffset = glm::translate(glm::mat4(1), glm::vec3(-0.5f, -0.5f, 0));
        m_surfaceTransform = surfaceRotation * surfaceScale * surfaceOffset  ;
    }
}

bool WaylandSurfaceNode::computeLocalSurfaceIntersection(const Geometry::Ray &localRay, glm::vec2 &localIntersection, float &t) const
{
    Geometry::Plane surfacePlane = Geometry::Plane(glm::vec3(0), glm::vec3(0,0,1));
    if(glm::dot(localRay.d, surfacePlane.n) == 0) return false;

    Geometry::Ray transformedRay = localRay.transform(glm::inverse(surfaceTransform()));

    t = surfacePlane.intersect(transformedRay);
    glm::vec3 intersection = transformedRay.solve(t);

    glm::vec3 coords= intersection * glm::vec3(m_surface->size().width(), m_surface->size().height(), 0);
    localIntersection =  glm::vec2(coords);

    return true;
}

WaylandSurfaceNode::RaySurfaceIntersection *WaylandSurfaceNode::intersectWithSurfaces(const Geometry::Ray &ray)
{
    WaylandSurfaceNode::RaySurfaceIntersection *closestSubtreeIntersection = SceneGraphNode::intersectWithSurfaces(ray);
    Geometry::Ray localRay = ray.transform(inverseTransform());

    float t;
    glm::vec2 localIntersection;
    bool isIntersected = computeLocalSurfaceIntersection(localRay, localIntersection, t);


    if(isIntersected && (closestSubtreeIntersection == NULL || t < closestSubtreeIntersection-> t)){

        if(localIntersection.x >= 0 && localIntersection.x <= m_surface->size().width() && localIntersection.y >= 0 && localIntersection.y <= m_surface->size().height){
            return new SceneGraphNode::RaySurfaceIntersection(this, localIntersection, ray, t);
        }else{
            return NULL;
        }

    }else{
        return closestSubtreeIntersection;
    }
}

void WaylandSurfaceNode::drawViewpoint(GLCamera *viewpoint)
{
}
