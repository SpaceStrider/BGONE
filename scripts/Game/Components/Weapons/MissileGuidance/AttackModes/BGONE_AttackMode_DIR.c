[BaseContainerProps()]
class BGONE_AttackProfile_DIR : BGONE_AttackProfile_Base
{
	
	void BGONE_AttackProfile_DIR()
	{
		m_cProfileName = "Direct Attack";
	}
	
	override BGONE_TargetData ProcessFrame(BGONE_TargetData targetData, float flightTime)
	{
		if (targetData.targetPosition == Vector(0,0,0))
		 	return targetData;

		/* Climb from ACE linear attack profile,
		vector projectilePos = m_eProjectile.GetOrigin();
		
		float distanceFromShooter = vector.Distance(targetData.launchPos, m_eProjectile.GetOrigin());
		float distanceToTarget = vector.Distance(targetData.targetPosition, m_eProjectile.GetOrigin());
		float totalDistance = vector.Distance(targetData.launchPos,targetData.targetPosition);
		
		vector addHeightVector = Vector(0,0,0);
		
		if ((projectilePos[1] - targetData.launchPos[1] < 5) && distanceFromShooter < 15)
		    addHeightVector = addHeightVector + (vector.Up * distanceToTarget);
		else if ((projectilePos[1] < targetData.targetPosition[1]) && distanceToTarget > 100)
			addHeightVector = addHeightVector + (vector.Up * (targetData.targetPosition[1] - projectilePos[1]));
		    
		if (projectilePos[1] > targetData.targetPosition[1])
		    addHeightVector = addHeightVector + (vector.Up * (distanceToTarget / 50));
		
		targetData.targetPosition = targetData.targetPosition + addHeightVector;
		*/
		
		return targetData;
	}		
	
};
