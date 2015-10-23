#include "Precompiled.h"

//=============================================================================
// EXTERNAL DECLARARIONS
//=============================================================================
#include "GoPlayer.h"

#include "Engine/Components/InputComponent.h"
#include "Engine/Components/SimulationComponent.h"
#include "Engine/Components/RenderComponent.h"
#include "Engine/Components/AIComponent.h"
#include "Engine/Components/PlayerComponent.h"
#include "Engine/Managers/SpawnManager.h"
#include "Import/DotX/DotXModel.h"
#include "Assets/Models/ModelRendering.h"
#include "Core/GameObjects/IGameObjectData.h"
#include "Core/GameObjects/IGameObject.h"
#include "Core/GameManagers/IGameRendering.h"
#include "Engine/GameObjects/GameObject.h"
#include "Core/GameObjects/GameObjectFactory.h"
#include "CollisionTypes.h"

#include "PxPhysicsAPI.h"
#include <d3dx9.h>



using namespace physx;

//=============================================================================
// CLASS GoPlayerData
//=============================================================================
class GoPlayerData: public IGameObjectData
{
public:
	// physics
	PxMaterial *_material;
	PxShape *_actorShape;

	// rendering
	DotXModel *_model;

public:
	GoPlayerData()
		: _material(nullptr)
		, _model(nullptr)
	{}

	int load()
	{
		//---------------------------------------------------------------------
		// create the physic object
		PxPhysics &physics = Game<IGameSimulation>()->physics();

		//static friction, dynamic friction, restitution
		_material = physics.createMaterial(0.5f, 0.5f, 0.1f); 

		//---------------------------------------------------------------------
		// create the render object
		_model = new DotXModel(Game<IGameRendering>()->d3dDevice(), "assets/ship.x", "assets/");

		return 0;
	}
};




//=============================================================================
// CLASS GoPlayerImp
//=============================================================================
class GoPlayerImp: public GoPlayer, public GameObject<GoPlayerImp, GoPlayerData>
{
public:
	// physics
	PxShape *_actorShape;

public:
	GoPlayerImp(const IGameObjectDataRef &aDataRef)
		: GameObject(aDataRef)
		, _actorShape(nullptr)
	{}

public:
	//-------------------------------------------------------------------------
	//
	static IGameObject::IdType TypeId()
	{
		return "GoPlayer";
	}

	//-------------------------------------------------------------------------
	//
	static IGameObjectData* loadData()
	{
		GoPlayerData *data = new GoPlayerData();
		data->load();
		return data;
	}

	//-------------------------------------------------------------------------
	//
	virtual void onSpawn(const PxTransform &aPose) override
	{
		// create the physic object

		auto simulationComponent = addComponent<DynamicSimulationComponent>();
		PxRigidDynamic &pxActor = simulationComponent->pxActor();
		pxActor.setGlobalPose(aPose);

		PxShape *actorShape = pxActor.createShape(PxSphereGeometry(0.5f), *_data->_material);
		pxActor.setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);


		PxFilterData filterData;
		filterData.word0 = eACTOR_PLAYER;
		filterData.word1 = eACTOR_ENEMY | eACTOR_TERRAIN;
		actorShape->setSimulationFilterData(filterData);


		//----------------------------------------------------------
		// create the render object

		// create the vertices using the CUSTOMVERTEX struct
		addComponent<RenderComponent>()->setRenderPrimitive(IRenderPrimitiveRef(new ModelRendering(*_data->_model)));

	}

	//-------------------------------------------------------------------------
	//
	virtual void onUnspawn() override
	{
	}
};

//-----------------------------------------------------------------------------
//
IGameObject::IdType GoPlayer::TypeId()
{	return GoPlayerImp::TypeId(); }

RegisterGameObjectType<GoPlayerImp> gRegisterActor;
