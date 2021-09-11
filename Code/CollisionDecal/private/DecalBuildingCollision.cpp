// Fill out your copyright notice in the Description page of Project Settings.


#include "DecalBuildingCollision.h"
#include "FoliageActors.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FoliageActors.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

//Constructor of the DecalBuildingCollision
UDecalBuildingCollision::UDecalBuildingCollision()
{
    //Establishing the mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TriggerMesh(TEXT("/Game/StarterContent/Shapes/Shape_Plane.Shape_Plane"));
    //Establishing the material we use when we can build
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> onMaterialToSet(TEXT("/Game/Materials/M_Ghost_CanBuilding.M_Ghost_CanBuilding"));
    //Establishing the material we use when we cant build
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> offMaterialToSet(TEXT("/Game/Materials/M_Ghost_CantBuilding.M_Ghost_CantBuilding"));
    this->SetStaticMesh(TriggerMesh.Object);
    //Establishing the collision on "Trigger"
    this->BodyInstance.SetCollisionProfileName(TEXT("Trigger"));
    //Establishing the scale of the mesh
    this->SetRelativeScale3D(FVector(3.25f, 3.25f, 1.f));
    OnMaterial = onMaterialToSet.Object;
    OffMaterial = offMaterialToSet.Object;
}

//We need to call this function in the begin play of the Owner actor.
void UDecalBuildingCollision::Initialise() 
{    
    if(OnMaterial == nullptr) return;
    this->SetMaterial(0, OnMaterial);
    //We are settings the Bounds of our CollisionDecal
    UKismetSystemLibrary::GetComponentBounds(
    this, 
    Origin, 
    BoxExtent, 
    Sphere);
    return;
}

//Function for removing the colliding Foliage
void UDecalBuildingCollision::RemoveOverlappingFoliageActors()
{
    if (IsOverlappingFoliageActors()){
        for (auto& actor : overlappingFoliageActors)
        {
            //DebugOption
            //GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, actor->GetName());
            actor->Destroy();

        }
        overlappingFoliageActors.Empty();
    }
}

//Function to check for foliage Colliding
bool UDecalBuildingCollision::IsOverlappingFoliageActors()
{
    GetOverlappingActors(overlappingFoliageActors, AFoliageActors::StaticClass());
    return overlappingFoliageActors.Num() > 0;
}

//Function to check for actors Colliding
bool UDecalBuildingCollision::IsOverlappingActors()
{
    GetOverlappingActors(overlappingActors);
    return overlappingActors.Num() > 0;
}

//Function to change the color if we can build or not (We need to pass the Mesh of owner)
void UDecalBuildingCollision::ChangeColor(UStaticMeshComponent * BuildingStaticMeshToSet)
{
    //We will check if we have null pointers 
    if (OnMaterial == nullptr) return;
	if (OffMaterial == nullptr) return;
    if (BuildingStaticMeshToSet == nullptr) return;
    BuildingStaticMesh = BuildingStaticMeshToSet;
    // If we can build we will set the OnMaterial. I we cant build we will set the offMaterial.
    if (CanBuild()) {
        this->SetMaterial(0, OnMaterial);
        BuildingStaticMesh->SetMaterial(0, OnMaterial);
    } else {
        this->SetMaterial(0, OffMaterial);
        BuildingStaticMesh->SetMaterial(0, OffMaterial);
    }
}

//Function to check if we can build or not
bool UDecalBuildingCollision::CanBuild()
{
    //We will check if we have null pointers 
    if (BuildingStaticMesh == nullptr) {return false;}
    //If we have Overlappong Actors we will return false
    if (IsOverlappingActors()){
        return false;
    } else {
        return CalcualteSlope();
    }
    return true;
}

//Function to get the actorLocation, forwardVector and rightVector from the actor.
void UDecalBuildingCollision::GetVectors(FVector & actorLocation, FVector & forwardVector, FVector & rightVector) const
{
    actorLocation = this->GetOwner()->GetActorLocation();
    forwardVector = this->GetOwner()->GetActorForwardVector();
    rightVector = this->GetOwner()->GetActorRightVector();
}
//Function to check if we are on a slope or we have a valid land to build.
bool UDecalBuildingCollision::CalcualteSlope()
{
    //Declare the variables we will use in our code 
    FVector actorLocation;
    FVector forwardVector;
    FVector rightVector;
    TArray<float> Differences;
    int32 minIndex = 0;
    int32 maxIndex = 0;
    float minValue = 0.f;
    float maxValue = 0.f;
    //This variable is the max distance we where can build from the land
    float maxDifferences = 20.f;
    GetVectors(actorLocation, forwardVector, rightVector);
    //We calculate the LineTracing in all corners of the DecalMesh and we add the differences between the corner and the land in the array.
    Differences.Add(SlopeTrace( ( (forwardVector+rightVector) * BoxExtent) + actorLocation) );
    Differences.Add(SlopeTrace( ( ((forwardVector.operator*(-1)) + rightVector) * BoxExtent) + actorLocation));
    Differences.Add(SlopeTrace( ( ((rightVector.operator*(-1)) + forwardVector) * BoxExtent) + actorLocation));
    Differences.Add(SlopeTrace( ( ((rightVector.operator*(-1)) + (forwardVector.operator*(-1)) ) * BoxExtent) + actorLocation));
    //We calculate the LineTracing in the middle of the DecalMesh and we add the difference between the middle and the land in the array.
    Differences.Add(SlopeTrace(actorLocation));

    //Debuggind code for the array.
    /*for (auto& value : Differences)
    {
        UE_LOG(LogTemp, Warning, TEXT("Valor:%f\n"), value);
    }*/
    //Minvalue in the array.
    minValue = FMath::Min<float>(Differences, &minIndex);
    //Maxvalue in the array.
    maxValue = FMath::Max<float>(Differences, &maxIndex);
    Differences.Empty();
    //return if we have a valid land to build.
    return (FMath::Abs(minValue)<maxDifferences || FMath::Abs(maxValue)<maxDifferences);
}
//Function to calculate the LineTracing from a TraceLocation.
float UDecalBuildingCollision::SlopeTrace(FVector TraceLocation)
{
    FHitResult outHit;
    FVector start = TraceLocation + FVector(0,0,1000);
    FVector end = TraceLocation - FVector(0,0,1000);
    GetWorld()->LineTraceSingleByChannel(outHit,start,end,ECollisionChannel::ECC_Visibility);
    return GetOwner()->GetActorLocation().Z - outHit.ImpactPoint.Z;
}
    