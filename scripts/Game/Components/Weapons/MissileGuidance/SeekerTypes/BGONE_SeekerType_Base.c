

[BaseContainerProps()]
class BGONE_SeekerType_Base
{
	protected RplComponent rplComponent;
	protected Projectile m_eProjectile;
	
	void InitSeeker(Projectile projectile, BGONE_TargetData targetData)
	{
		m_eProjectile = projectile;
		rplComponent = RplComponent.Cast(m_eProjectile.FindComponent(RplComponent));
	}
	
 	array<int> GetAvailableArmingDistances()
	{
		return {0};
	}
	
	BGONE_TargetData ProcessFrame(BGONE_TargetData targetData, float flightTime)
	{
		
	}
	
	protected float GetDistanceFromLaunch(BGONE_TargetData targetData)
	{
		return vector.Distance(targetData.launchPos, m_eProjectile.GetOrigin());
	}
};
