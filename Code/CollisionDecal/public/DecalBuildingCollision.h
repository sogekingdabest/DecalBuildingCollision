// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "DecalBuildingCollision.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class COLLISIONCONSTRUCTIO_API UDecalBuildingCollision : public UStaticMeshComponent
{
	GENERATED_BODY()
	
	public:
	//Array for overlappingActors
	TArray<AActor*> overlappingActors;

	//Array for overlappingFoliageActors
	TArray<AActor*> overlappingFoliageActors;

	//StaticMesh of the Owner
	class UStaticMeshComponent * BuildingStaticMesh;

	//Variables for the Function GetVectors().
	FVector Origin;

	FVector BoxExtent;
	
	float Sphere;

	UPROPERTY(EditAnywhere, Category = "CollisionSetup")
	class UMaterialInterface* OnMaterial;

	UPROPERTY(EditAnywhere, Category = "CollisionSetup")
	class UMaterialInterface* OffMaterial;

	UDecalBuildingCollision();

	UFUNCTION(BlueprintCallable, Category = "CollisionSetup")
	void RemoveOverlappingFoliageActors();

	UFUNCTION(BlueprintCallable, Category = "CollisionSetup")
	void Initialise();

	UFUNCTION(BlueprintCallable, Category = "CollisionSetup")
	void ChangeColor(UStaticMeshComponent * BuildingStaticMeshToSet);

	UFUNCTION(BlueprintCallable, Category = "CollisionSetup")
	bool CanBuild();

	bool IsOverlappingFoliageActors();

	bool IsOverlappingActors();

	void GetVectors(FVector & actorLocation, FVector & forwardVector, FVector & rightVector) const;

	bool CalcualteSlope();

	float SlopeTrace(FVector TraceLocation);

};
