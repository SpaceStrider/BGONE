
[BaseContainerProps()]
class BGONE_SeekerType_PLOS : BGONE_SeekerType_Base
{
	[Attribute("20", UIWidgets.Auto, "Possible Selections Of Arming Distances Until The Magnetic Proximity Sensor Is Activated (Only In FlyOver Mode).","0 10000", category: "BGONE")]
	protected ref array<int> m_fArmingDistances;
	
	[Attribute("5", UIWidgets.Auto, "How Far Down Can The Proximity Sensor Detect Targets (Only In FlyOver Mode).","0 10000", category: "BGONE")]
	protected float m_fProximityDetectionRange;

	protected int m_iArmingDistanceIndex = 0;
	BGONE_TargetData m_eTargetData;
	protected bool m_bIsFlyover;
	
	override void InitSeeker(Projectile projectile, BGONE_TargetData targetData)
	{
		super.InitSeeker(projectile, targetData);
		 
		BGONE_GuidedMissileComponent missileComponent = BGONE_GuidedMissileComponent.Cast(projectile.FindComponent(BGONE_GuidedMissileComponent));
		m_bIsFlyover = missileComponent.GetActiveAttackProfile().Type() == BGONE_AttackProfile_PLOS_FLYOVER;
		
		m_iArmingDistanceIndex = targetData.armingDistancesIndex;
	}
	
	override array<int> GetAvailableArmingDistances()
	{
		return m_fArmingDistances;
	}
	
	override BGONE_TargetData ProcessFrame(BGONE_TargetData targetData, float flightTime)
	{	
		if(targetData.detonated > 0)
		{
			if(m_bIsFlyover)
				targetData.detonated = 2;
			return targetData;
		}
		
		if(m_bIsFlyover && GetDistanceFromLaunch(targetData) > m_fArmingDistances[m_iArmingDistanceIndex])
			targetData.detonated = ProcessProxyFuse();
		
		m_eTargetData = targetData;
		return m_eTargetData;
	}
	
	protected int ProcessProxyFuse()
	{
		bool detonate = TopDownTracer(m_eProjectile.GetOrigin(), m_fProximityDetectionRange, m_eProjectile);
		if(!detonate)
			return 0;
			
		return 2;
	}

	protected bool TopDownTracer(vector vStartPosition, float fTravelDistance, IEntity owner = null, IEntity target = null) 
	{
		TraceParam traceparams = TraceParam();
		traceparams.Flags = TraceFlags.ENTS;
		traceparams.LayerMask = EPhysicsLayerDefs.Projectile;
		ref array<IEntity> excludeArray = {owner, target};
		traceparams.ExcludeArray = excludeArray;
		traceparams.Start = vStartPosition;
		traceparams.End = traceparams.Start + fTravelDistance * vector.Up * -1;
		
		BaseWorld world = owner.GetWorld();
		world.TraceMove(traceparams, CheckIfIsVehicle);
		return traceparams.TraceEnt != null;
	}
	
	protected bool CheckIfIsVehicle(IEntity ent)
	{
		if (!ent)
			return false;

		BaseVehicle isVehicle = BaseVehicle.Cast(ent);
		if (!isVehicle)
			return false;

		return true;
	}
};
