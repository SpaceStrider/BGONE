[BaseContainerProps()]
class BGONE_AttackProfile_PLOS_FLYOVER : BGONE_AttackProfile_PLOS
{
	void BGONE_AttackProfile_PLOS_FLYOVER()
	{
		m_cProfileName = "FlyOver";
	}
	
	[Attribute("2", UIWidgets.Slider, "How Many Meters Above The Target Should The Missile Fly In FlyOver Mode", "0 10 0.1", category: "BGONE")]
	protected float m_fFlyOverOffset;
	
	[Attribute("20", UIWidgets.Slider, "Distance Until The FlyOver Offset Is Reached", "0 1000 1", category: "BGONE")]
	protected int m_fFlyOverOffsetRange;
	
	override BGONE_TargetData ProcessFrame(BGONE_TargetData targetData, float flightTime)
	{
		targetData = super.ProcessFrame(targetData, flightTime);
		targetData.targetPosition += (vector.Up * (Math.Lerp(0,m_fFlyOverOffset,Math.Map(Math.Clamp(GetDistanceFromLaunch(targetData),0,m_fFlyOverOffsetRange),0,m_fFlyOverOffsetRange,0,1))));
		
		return targetData;
	}		
	
};
