[BaseContainerProps()]
class BGONE_AttackProfile_TOP : BGONE_AttackProfile_Base
{	
	[Attribute("160", UIWidgets.Slider, "Cruise Altitude", "0 1000 1", category: "BGONE")]
	protected int m_fCruiseAltitude;
	[Attribute("1250", UIWidgets.Slider, "How Far The Target Must Be From The Shooter For Cruise Altitude To Be Used", "0 10000 1", category: "BGONE")]
	protected int m_fCruiseAltitudeRange;
	
	protected int m_iCurrentStage = 0;
	
	void BGONE_AttackProfile_TOP()
	{
		m_cProfileName = "Top Attack";
	}
	
	override BGONE_TargetData ProcessFrame(BGONE_TargetData targetData, float flightTime)
	{
		if (targetData.targetPosition == Vector(0,0,0))
		 	return targetData;
		
		
		float distanceFromShooter = vector.Distance(targetData.launchPos, m_eProjectile.GetOrigin());
		float distanceToTarget = vector.Distance(targetData.targetPosition, m_eProjectile.GetOrigin());
		float totalDistance = vector.Distance(targetData.launchPos,targetData.targetPosition);

		switch(m_iCurrentStage)
		{
			// Launch
			case 0:
			{
				if (distanceFromShooter < 10) 
				{
		            targetData.targetPosition = targetData.targetPosition + vector.Up * distanceToTarget * 2;
		        } 
				else {
			    	m_iCurrentStage = 1;
		        };
				break;
			}
			// Climb
			case 1:
			{
				float currentCruiseAlt = m_fCruiseAltitude;
				if(totalDistance < m_fCruiseAltitudeRange)
				{
					currentCruiseAlt = m_fCruiseAltitude * (totalDistance/m_fCruiseAltitudeRange);
				}
	       		if(m_eProjectile.GetOrigin()[1] - targetData.targetPosition[1] >= currentCruiseAlt) 
				{
					if(currentCruiseAlt < m_fCruiseAltitude)
	            		m_iCurrentStage = 3;
					else
						m_iCurrentStage = 2;
	        	} 
				else 
				{
	             	targetData.targetPosition = targetData.targetPosition + vector.Up * distanceToTarget * 1.5;
	        	};
				break;
			}
			// Coast
			case 2:
			{
				if(distanceToTarget < (m_eProjectile.GetOrigin()[1] - targetData.targetPosition[1]) * 2)
					m_iCurrentStage = 3;
				else
					targetData.targetPosition = targetData.targetPosition + vector.Up * m_eProjectile.GetOrigin()[1];
				break;
			}
			// Terminal
			case 3:
			{
				targetData.targetPosition = targetData.targetPosition;
				break;
			}
		}
		
		return targetData;
	}		
	
};
