class BGONE_ArtilleryComputerComponentClass : ScriptGameComponentClass
{

}

class BGONE_ArtilleryComputerComponent : ScriptComponent
{
	protected bool m_bIsFirstTimeOpened = true;
	protected SCR_MapEntity m_MapEntity;
	protected IEntity m_eOwner;
	protected TurretComponent m_Turret;
	protected TurretControllerComponent m_TurretController;
	protected vector limitsHoriz, limitsVert;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_eOwner = owner;
	}
	void OpenComputer(SCR_MapEntity mapEntity, IEntity user)
	{
		SCR_ChimeraCharacter userCharacter = SCR_ChimeraCharacter.Cast(user);
		m_TurretController = TurretControllerComponent.Cast(userCharacter.GetCompartmentAccessComponent().GetCompartment().GetController());
		if(!m_TurretController)
			return;
		
		// Todo, check aiming limits.
		m_TurretController.GetTurretComponent().GetAimingLimits(limitsHoriz, limitsVert);
		
		m_MapEntity = mapEntity;
		
		m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		m_MapEntity.GetOnMapClose().Insert(OnMapClose);
		m_MapEntity.GetOnSelection().Insert(OnMapSelection);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.MB_ArtilleryComputer);
	}
	protected float CalculatePitchAngle(float range, float elevationDifference, float initialVelocity)
    {
		float gravity = PhysicsWorld.GetGravity(GetGame().GetWorldEntity())[1] * -1;
        
		Math.Pow(initialVelocity,4) - (gravity * ((gravity * Math.Pow(range,2)) + (2 * elevationDifference * Math.Pow(initialVelocity,2))));
        float term2 = (gravity * Math.Pow(range, 2)) - (2 * elevationDifference * Math.Pow(initialVelocity, 2));

        // Calculate the discriminant
        float discriminant = Math.Pow(initialVelocity, 2) + (gravity * term2);
        // Check if the projectile can reach the target
        if (discriminant < 0)
        {
            Print("Cant Reach Target Distance");
			return 45 * Math.DEG2RAD;
        }
			
        float numerator = initialVelocity * initialVelocity + Math.Sqrt(discriminant);
        float denominator = gravity * range;

        // Calculate the pitch angle in radians
        float pitchAngleRadians = Math.Atan2(numerator, denominator);

        return pitchAngleRadians;
    }
	protected void CenterMapOnVehicle()
	{
		vector o = m_eOwner.GetOrigin();
		float x, y;
		m_MapEntity.WorldToScreen(o[0], o[2], x, y);
		m_MapEntity.PanSmooth(x, y);
	}
	void OnMapSelection(vector selectedPos)
	{
		float wX, wY;
		m_MapEntity.ScreenToWorld(selectedPos[0],selectedPos[2],wX,wY);
		float heightAtPos = m_MapEntity.GetWorld().GetSurfaceY(wX,wY);
		vector worldTarget = Vector(wX,heightAtPos,wY);
		
		vector currentPos = m_eOwner.GetOrigin();
		float distance = vector.DistanceXZ(currentPos, worldTarget);
		float elevDiff = currentPos[1] + 3 - worldTarget[1];
		Print("Selected Pos : " + worldTarget);
		Print("Distance : " + distance);
		Print("Elevation Difference : " + elevDiff);
		
		
		float aimPitch = CalculatePitchAngle(distance,elevDiff,150);
		Print("Required Angle :" + aimPitch * Math.RAD2DEG);
		
		vector aimDir = worldTarget - currentPos;
		float wantedYaw = aimDir.ToYaw();
		float yawDiff = wantedYaw - m_eOwner.GetYawPitchRoll()[0];
		m_TurretController.GetTurretComponent().SetAimingRotation(Vector(yawDiff * Math.DEG2RAD,aimPitch,0));
		Print("AimDir: " + m_TurretController.GetTurretComponent().GetAimingDirectionWorld().VectorToAngles());
	}
	
	protected void OnMapOpen(MapConfiguration config)
	{
		if (!m_bIsFirstTimeOpened)
			return;
		
		m_bIsFirstTimeOpened = false;
			
		GetGame().GetCallqueue().CallLater(CenterMapOnVehicle);
	}
	
	protected void OnMapClose(MapConfiguration config)
	{
		m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		m_MapEntity.GetOnMapClose().Remove(OnMapClose);
	}
}