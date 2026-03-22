#ifndef _COLLISIONCLASS_H_
#define _COLLISIONCLASS_H_

#include <directxmath.h>
#include <vector>
#include <string>
#include <cfloat>
#include <algorithm>

#include "jsonSettingsLoader.h"
#include "modelclass.h"

using namespace DirectX;
using namespace std;

class CollisionClass
{
public:
	// AABB 구조체
	struct AABB
	{
		XMFLOAT3 min;
		XMFLOAT3 max;

		AABB() : min(0 , 0 , 0) , max(0 , 0 , 0) {}
		AABB(XMFLOAT3 minPoint , XMFLOAT3 maxPoint) : min(minPoint) , max(maxPoint) {}
	};

	// 인스턴스별 Collider 정보
	struct InstanceCollider
	{
		AABB aabb;
		bool enableCollision;
		int instanceIndex;
	};

	// Collider 정보
	struct ColliderInfo
	{
		AABB originalAABB;           // 원본 모델의 AABB
		AABB currentAABB;            // 메인 인스턴스의 현재 AABB
		vector<InstanceCollider> instanceColliders; // 인스턴스별 충돌체들
		bool isStatic;               // 정적 객체인지 여부
		bool enableCollision;        // 충돌 검사 활성화 여부
		bool hasInstances;           // 인스턴스를 가지는지 여부
		string modelName;            // 모델 이름 (디버깅용)
	};

public:
	CollisionClass();
	CollisionClass(const CollisionClass&);
	~CollisionClass();

	bool Initialize();
	void Shutdown();

	// 모델 Collider 설정
	bool SetupModelColliders(const vector<ModelConfig>& modelConfigs , vector<ModelClass*>& models);

	// AABB 계산
	AABB CalculateModelAABB(ModelClass* model);
	AABB ApplyTransformToAABB(const AABB& originalAABB , const XMMATRIX& worldMatrix);

	// Collider 업데이트
	void UpdateCollider(int modelIndex , const XMMATRIX& worldMatrix);
	void UpdateAllStaticColliders(const vector<XMMATRIX>& worldMatrices);
	void UpdateInstanceColliders(int modelIndex , const vector<XMMATRIX>& instanceMatrices);

	// 충돌 검사
	bool CheckCollision(int modelA , int modelB);
	bool CheckInstanceCollision(int modelA , int instanceA , int modelB , int instanceB = -1);
	bool CheckAABBCollision(const AABB& a , const AABB& b);

	// 특정 모델과의 충돌 검사
	vector<int> CheckCollisionsWith(int modelIndex);
	vector<pair<int , int>> CheckCollisionsWithInstances(int modelIndex);
	bool CheckPointInAABB(const XMFLOAT3& point , const AABB& aabb);

	// 디버그 및 정보
	AABB GetColliderAABB(int modelIndex) const;
	void SetCollisionEnabled(int modelIndex , bool enabled);
	bool IsCollisionEnabled(int modelIndex) const;

	// 디버그 정보를 위한 getter
	const vector<ColliderInfo>& GetAllColliders() const { return m_colliders; }

	bool TestDetailedCollision(int modelA , int modelB) const;

	// 충돌 시스템 전체 활성화/비활성화 담당
	void ToggleCollisionSystem() { m_isCollisionSystemEnabled = !m_isCollisionSystemEnabled; }
	bool IsCollisionSystemEnabled() const { return m_isCollisionSystemEnabled; }

private:
	vector<ColliderInfo> m_colliders;
	int m_modelCount;
	bool m_isCollisionSystemEnabled;

	// 유틸리티 함수들
	XMFLOAT3 TransformPoint(const XMFLOAT3& point , const XMMATRIX& matrix);
	void ExpandAABBWithPoint(AABB& aabb , const XMFLOAT3& point);
	void ResetAABB(AABB& aabb);
};

#endif