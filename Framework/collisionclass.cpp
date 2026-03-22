#include "collisionclass.h"

CollisionClass::CollisionClass()
{
	m_modelCount = 0;
	m_isCollisionSystemEnabled = true;
}

CollisionClass::CollisionClass(const CollisionClass& other)
{
}

CollisionClass::~CollisionClass()
{
}

bool CollisionClass::Initialize()
{
	return true;
}

void CollisionClass::Shutdown()
{
	m_colliders.clear();
}

bool CollisionClass::SetupModelColliders(const vector<ModelConfig>& modelConfigs , vector<ModelClass*>& models)
{
	m_modelCount = static_cast< int >( models.size() );
	m_colliders.resize(m_modelCount);

	for ( int i = 0; i < m_modelCount; i++ )
	{
		if ( models[ i ] != nullptr )
		{
			// 원본 AABB 계산
			m_colliders[ i ].originalAABB = CalculateModelAABB(models[ i ]);
			m_colliders[ i ].currentAABB = m_colliders[ i ].originalAABB;

			// 모델 정보 설정
			if ( i < modelConfigs.size() )
			{
				m_colliders[ i ].modelName = modelConfigs[ i ].name;

				// 인스턴스 정보 설정
				if ( modelConfigs[ i ].instanceCount > 1 )
				{
					m_colliders[ i ].hasInstances = true;
					m_colliders[ i ].instanceColliders.resize(modelConfigs[ i ].instanceCount);

					// 각 인스턴스별 충돌체 초기화
					for ( unsigned int j = 0; j < modelConfigs[ i ].instanceCount; j++ )
					{
						m_colliders[ i ].instanceColliders[ j ].aabb = m_colliders[ i ].originalAABB;
						m_colliders[ i ].instanceColliders[ j ].enableCollision = true;
						m_colliders[ i ].instanceColliders[ j ].instanceIndex = j;
					}
				}
				else
				{
					m_colliders[ i ].hasInstances = false;
				}
			}
			else
			{
				m_colliders[ i ].modelName = "Unknown_" + to_string(i);
				m_colliders[ i ].hasInstances = false;
			}

			// 충돌 설정 수정
			if ( i >= 1 && i <= 6 ) // 차량 모델들
			{
				m_colliders[ i ].enableCollision = true;
				m_colliders[ i ].isStatic = false;
			}
			else if ( i == 0 || i == 7 || i == 15 || i == 16 ) // 충돌 제외 모델들
			{
				m_colliders[ i ].enableCollision = false;
				m_colliders[ i ].isStatic = true;
			}
			else // 나머지 모든 모델들
			{
				m_colliders[ i ].enableCollision = true;
				m_colliders[ i ].isStatic = true;
			}
		}
		else
		{
			m_colliders[ i ].enableCollision = false;
			m_colliders[ i ].isStatic = true;
			m_colliders[ i ].hasInstances = false;
			m_colliders[ i ].modelName = "NULL_MODEL";
		}
	}

	return true;
}

CollisionClass::AABB CollisionClass::CalculateModelAABB(ModelClass* model)
{
	AABB aabb;
	ResetAABB(aabb);

	if ( !model )
		return aabb;

	ModelClass::VertexType* vertices = model->GetVertices();
	unsigned int vertexCount = model->GetVertexCount();

	if ( !vertices || vertexCount == 0 )
		return aabb;

	aabb.min = vertices[ 0 ].position;
	aabb.max = vertices[ 0 ].position;

	for ( unsigned int i = 1; i < vertexCount; i++ )
	{
		const XMFLOAT3& pos = vertices[ i ].position;

		aabb.min.x = min(aabb.min.x , pos.x);
		aabb.min.y = min(aabb.min.y , pos.y);
		aabb.min.z = min(aabb.min.z , pos.z);

		aabb.max.x = max(aabb.max.x , pos.x);
		aabb.max.y = max(aabb.max.y , pos.y);
		aabb.max.z = max(aabb.max.z , pos.z);
	}

	return aabb;
}

CollisionClass::AABB CollisionClass::ApplyTransformToAABB(const AABB& originalAABB , const XMMATRIX& worldMatrix)
{
	XMFLOAT3 vertices[ 8 ] = {
		{originalAABB.min.x, originalAABB.min.y, originalAABB.min.z},
		{originalAABB.max.x, originalAABB.min.y, originalAABB.min.z},
		{originalAABB.min.x, originalAABB.max.y, originalAABB.min.z},
		{originalAABB.max.x, originalAABB.max.y, originalAABB.min.z},
		{originalAABB.min.x, originalAABB.min.y, originalAABB.max.z},
		{originalAABB.max.x, originalAABB.min.y, originalAABB.max.z},
		{originalAABB.min.x, originalAABB.max.y, originalAABB.max.z},
		{originalAABB.max.x, originalAABB.max.y, originalAABB.max.z}
	};

	AABB transformedAABB;
	ResetAABB(transformedAABB);

	XMFLOAT3 firstTransformed = TransformPoint(vertices[ 0 ] , worldMatrix);
	transformedAABB.min = firstTransformed;
	transformedAABB.max = firstTransformed;

	for ( int i = 1; i < 8; i++ )
	{
		XMFLOAT3 transformed = TransformPoint(vertices[ i ] , worldMatrix);
		ExpandAABBWithPoint(transformedAABB , transformed);
	}

	return transformedAABB;
}

void CollisionClass::UpdateCollider(int modelIndex , const XMMATRIX& worldMatrix)
{
	if ( modelIndex < 0 || modelIndex >= m_modelCount )
		return;

	m_colliders[ modelIndex ].currentAABB = ApplyTransformToAABB(
		m_colliders[ modelIndex ].originalAABB , worldMatrix);
}

void CollisionClass::UpdateAllStaticColliders(const vector<XMMATRIX>& worldMatrices)
{
	for ( int i = 0; i < m_modelCount && i < worldMatrices.size(); i++ )
	{
		if ( m_colliders[ i ].isStatic && m_colliders[ i ].enableCollision )
		{
			UpdateCollider(i , worldMatrices[ i ]);
		}
	}
}

void CollisionClass::UpdateInstanceColliders(int modelIndex , const vector<XMMATRIX>& instanceMatrices)
{
	if ( modelIndex < 0 || modelIndex >= m_modelCount )
		return;

	if ( !m_colliders[ modelIndex ].hasInstances )
		return;

	// 각 인스턴스의 충돌체 업데이트
	for ( size_t i = 0; i < m_colliders[ modelIndex ].instanceColliders.size() && i < instanceMatrices.size(); i++ )
	{
		m_colliders[ modelIndex ].instanceColliders[ i ].aabb =
			ApplyTransformToAABB(m_colliders[ modelIndex ].originalAABB , instanceMatrices[ i ]);
	}
}

bool CollisionClass::CheckCollision(int modelA , int modelB)
{
	if ( modelA < 0 || modelA >= m_modelCount ||
		modelB < 0 || modelB >= m_modelCount ||
		modelA == modelB )
		return false;

	// 둘 중 하나라도 충돌이 비활성화되어 있으면 충돌하지 않음
	if ( !m_colliders[ modelA ].enableCollision || !m_colliders[ modelB ].enableCollision )
		return false;

	return CheckAABBCollision(m_colliders[ modelA ].currentAABB , m_colliders[ modelB ].currentAABB);
}

bool CollisionClass::CheckAABBCollision(const AABB& a , const AABB& b)
{
	return ( a.min.x <= b.max.x && a.max.x >= b.min.x ) &&
		( a.min.y <= b.max.y && a.max.y >= b.min.y ) &&
		( a.min.z <= b.max.z && a.max.z >= b.min.z );
}

bool CollisionClass::CheckInstanceCollision(int modelA , int instanceA , int modelB , int instanceB)
{
	if ( modelA < 0 || modelA >= m_modelCount || modelB < 0 || modelB >= m_modelCount )
		return false;

	if ( !m_colliders[ modelA ].enableCollision || !m_colliders[ modelB ].enableCollision )
		return false;

	AABB aabbA , aabbB;

	// 모델 A의 AABB 가져오기
	if ( instanceA >= 0 && m_colliders[ modelA ].hasInstances &&
		instanceA < m_colliders[ modelA ].instanceColliders.size() )
	{
		aabbA = m_colliders[ modelA ].instanceColliders[ instanceA ].aabb;
	}
	else
	{
		aabbA = m_colliders[ modelA ].currentAABB;
	}

	// 모델 B의 AABB 가져오기
	if ( instanceB >= 0 && m_colliders[ modelB ].hasInstances &&
		instanceB < m_colliders[ modelB ].instanceColliders.size() )
	{
		aabbB = m_colliders[ modelB ].instanceColliders[ instanceB ].aabb;
	}
	else
	{
		aabbB = m_colliders[ modelB ].currentAABB;
	}

	return CheckAABBCollision(aabbA , aabbB);
}

vector<int> CollisionClass::CheckCollisionsWith(int modelIndex)
{
	vector<int> collisions;

	if ( modelIndex < 0 || modelIndex >= m_modelCount || !m_colliders[ modelIndex ].enableCollision )
		return collisions;

	for ( int i = 0; i < m_modelCount; i++ )
	{
		if ( i != modelIndex && CheckCollision(modelIndex , i) )
		{
			collisions.push_back(i);
		}
	}

	return collisions;
}

vector<pair<int , int>> CollisionClass::CheckCollisionsWithInstances(int modelIndex)
{
	vector<pair<int , int>> collisions;

	if ( modelIndex < 0 || modelIndex >= m_modelCount || !m_colliders[ modelIndex ].enableCollision )
		return collisions;

	// 다른 모든 모델들과 충돌 검사
	for ( int i = 0; i < m_modelCount; i++ )
	{
		if ( i == modelIndex || !m_colliders[ i ].enableCollision )
			continue;

		// 일반 충돌 검사
		if ( CheckCollision(modelIndex , i) )
		{
			collisions.push_back({ i, -1 }); // -1은 메인 모델
		}

		// 인스턴스가 있는 경우 각 인스턴스와 충돌 검사
		if ( m_colliders[ i ].hasInstances )
		{
			for ( size_t j = 0; j < m_colliders[ i ].instanceColliders.size(); j++ )
			{
				if ( CheckInstanceCollision(modelIndex , -1 , i , static_cast< int >(j)) )
				{
					collisions.push_back({ i, static_cast< int >(j) });
				}
			}
		}
	}

	return collisions;
}

bool CollisionClass::CheckPointInAABB(const XMFLOAT3& point , const AABB& aabb)
{
	return ( point.x >= aabb.min.x && point.x <= aabb.max.x ) &&
		( point.y >= aabb.min.y && point.y <= aabb.max.y ) &&
		( point.z >= aabb.min.z && point.z <= aabb.max.z );
}

CollisionClass::AABB CollisionClass::GetColliderAABB(int modelIndex) const
{
	if ( modelIndex < 0 || modelIndex >= m_modelCount )
		return AABB();

	return m_colliders[ modelIndex ].currentAABB;
}

void CollisionClass::SetCollisionEnabled(int modelIndex , bool enabled)
{
	if ( modelIndex >= 0 && modelIndex < m_modelCount )
	{
		m_colliders[ modelIndex ].enableCollision = enabled;
	}
}

bool CollisionClass::IsCollisionEnabled(int modelIndex) const
{
	if ( modelIndex < 0 || modelIndex >= m_modelCount )
		return false;

	return m_colliders[ modelIndex ].enableCollision;
}

XMFLOAT3 CollisionClass::TransformPoint(const XMFLOAT3& point , const XMMATRIX& matrix)
{
	XMVECTOR pointVec = XMLoadFloat3(&point);
	XMVECTOR transformedVec = XMVector3TransformCoord(pointVec , matrix);

	XMFLOAT3 result;
	XMStoreFloat3(&result , transformedVec);
	return result;
}

void CollisionClass::ExpandAABBWithPoint(AABB& aabb , const XMFLOAT3& point)
{
	aabb.min.x = min(aabb.min.x , point.x);
	aabb.min.y = min(aabb.min.y , point.y);
	aabb.min.z = min(aabb.min.z , point.z);

	aabb.max.x = max(aabb.max.x , point.x);
	aabb.max.y = max(aabb.max.y , point.y);
	aabb.max.z = max(aabb.max.z , point.z);
}

void CollisionClass::ResetAABB(AABB& aabb)
{
	aabb.min = XMFLOAT3(FLT_MAX , FLT_MAX , FLT_MAX);
	aabb.max = XMFLOAT3(-FLT_MAX , -FLT_MAX , -FLT_MAX);
}

bool CollisionClass::TestDetailedCollision(int modelA , int modelB) const
{
	const AABB& a = m_colliders[ modelA ].currentAABB;
	const AABB& b = m_colliders[ modelB ].currentAABB;

	bool xOverlap = ( a.min.x <= b.max.x && a.max.x >= b.min.x );
	bool yOverlap = ( a.min.y <= b.max.y && a.max.y >= b.min.y );
	bool zOverlap = ( a.min.z <= b.max.z && a.max.z >= b.min.z );

	return xOverlap && yOverlap && zOverlap;
}