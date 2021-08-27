#include "CustomTypes/Bucket.hpp"

#include "ScribbleContainer.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Transform.hpp"
#include "logging.hpp"
DEFINE_TYPE(Scribble, Bucket);

using namespace UnityEngine;
namespace Scribble
{
    void Bucket::Awake()
    {
        set_enabled(false);
        bucketSize = 0.1f;
    }

    void Bucket::Update()
    {
        Sombrero::FastVector3 position = get_transform()->get_position();
        float dist = lastBucketPosition.sqrDistance(position);
        if (dist > 0.01f)
        {
            lastBucketPosition = position;
            ScribbleContainer::get_instance()->Bucket(lastBucketPosition, bucketSize, brushBehaviour->currentBrush);
        }
    }

    void Bucket::Init(Scribble::BrushBehaviour* brushBehaviour)
    {
        this->brushBehaviour = brushBehaviour;
    }

    void Bucket::StartBucketing()
    {
        brushBehaviour->brushMesh->get_transform()->set_localScale(Sombrero::FastVector3(bucketSize, bucketSize, bucketSize));
        brushBehaviour->brushMesh->GetComponent<MeshRenderer*>()->get_material()->set_color(bucketColor);
        brushBehaviour->brushMesh->SetActive(true);
        set_enabled(true);
    }

    void Bucket::StopBucketing()
    {
        brushBehaviour->brushMesh->SetActive(false);
        set_enabled(false);
    }
}