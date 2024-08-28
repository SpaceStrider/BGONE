
[BaseContainerProps()]
class BGONE_SeekerType_SACLOS : BGONE_SeekerType_Base
{
	[Attribute("5.6", UIWidgets.Slider, "How Many Seconds Until The Missile Self Destructs.","0 30 0.1", category: "BGONE")]
	protected float m_fTimeToLive;
	
	[Attribute("60", UIWidgets.Slider, "Degrees The Seeker Can See.","0 360 1", category: "BGONE")]
	protected float m_fSeekerAngle;
	
	protected float m_fDistanceFromLaunch;
	protected vector m_fProjectilePos;
	protected ref BGONE_TargetData m_eTargetData;
	
	protected vector ownerAimDir;
	protected vector ownerAimPos;
	protected float lastOwnerUpdate;
	
	protected SCR_ChimeraCharacter m_eShooter;
	protected TurretControllerComponent m_eTurret;
	
	override void InitSeeker(Projectile projectile, BGONE_TargetData targetData)
	{
		super.InitSeeker(projectile, targetData);
		m_eTargetData = targetData;
	}
	
	override BGONE_TargetData ProcessFrame(BGONE_TargetData targetData, float flightTime)
	{
		m_fProjectilePos = m_eProjectile.GetOrigin();
		m_fDistanceFromLaunch = vector.Distance(targetData.launchPos, m_fProjectilePos);
	
		SCR_ChimeraCharacter shooter = targetData.GetShooterEntity();
		
		vector aimDir;
		vector aimPos;
		
		// use updated values from owner, or fall back to server values if no update for 500 milliseconds.
		if(ownerAimDir && ownerAimPos)
		{
			aimDir = ownerAimDir;
			aimPos = ownerAimPos;
			
			ownerAimDir = vector.Zero;
			ownerAimPos = vector.Zero;
			lastOwnerUpdate = GetGame().GetWorld().GetWorldTime();
		}
		else if(!lastOwnerUpdate || GetGame().GetWorld().GetWorldTime() - lastOwnerUpdate > 500)
		{
			TurretControllerComponent turret = targetData.GetTurretEntity();
			
			// If launcher is in a turret
			if(turret)
			{
				aimDir = turret.GetCurrentSights().GetSightsDirectionUntransformed();
				aimPos = turret.GetCurrentSights().GetSightsRearPosition();
			}
			else
			{
				aimDir = shooter.GetHeadAimingComponent().GetAimingDirectionWorld();
				aimDir[0] = -aimDir[0];
				aimPos = shooter.EyePosition();
			}
		}
		
		vector directionNormal = vector.Direction(aimPos,m_fProjectilePos).Normalized();
		float dotProd = vector.Dot(aimDir.Normalized(),directionNormal);
		
		// Lost tracking due to projectile being obscured or seeker not seeing the shooter.
		bool losToProjectile = TraceLOS(aimPos, m_fProjectilePos, shooter);
		if(dotProd < Math.Cos(m_fSeekerAngle * Math.DEG2RAD) || !losToProjectile)
		{
			targetData.targetPosition = Vector(0,0,0); // Continue in a straight line
		}
		else 
		{
			targetData.targetPosition =  aimPos +  aimDir * (m_fDistanceFromLaunch + 10);
		}
		
		m_eTargetData = targetData;
		
		if(flightTime > m_fTimeToLive)
			m_eTargetData.detonated = 1;
		
		return m_eTargetData;
	}
	
	void UpdateAimingDirServer(vector aimDir, vector aimPos)
	{
		ownerAimDir = aimDir;
		ownerAimPos = aimPos;
	}
	
	protected bool TraceLOS(vector from, vector to, IEntity shooter)
	{						
		ref array<IEntity> exclude = {m_eProjectile, shooter };
		TraceParam param = new TraceParam;
		param.Start = from;
		param.End = to;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Flags = TraceFlags.ANY_CONTACT | TraceFlags.WORLD | TraceFlags.ENTS; 
		param.ExcludeArray = exclude;

		float percent = GetGame().GetWorld().TraceMove(param, null);
		if (percent == 1)	// If trace travels the entire path, return true
			return true;
				
		return false;
	}
	
};
