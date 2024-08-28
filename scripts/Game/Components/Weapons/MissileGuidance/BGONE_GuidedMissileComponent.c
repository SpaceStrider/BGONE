[ComponentEditorProps(category: "GameScripted/Weapons/BGONE", description: "")]
class BGONE_GuidedMissileComponentClass : ScriptGameComponentClass
{
};
class BGONE_GuidedMissileComponent : ScriptComponent
{
	[Attribute("", UIWidgets.Object, desc: "Script Responsible For Updating The Target During Flight - Also Handles Proximity Detonation", category: "BGONE")]
	protected ref BGONE_SeekerType_Base m_eSeekerTypeComponent;
	
	[Attribute("", UIWidgets.Object, desc: "Scripts Responsible For Adjusting The Attack Mode Of The Missile", category: "BGONE")]
	protected ref array<ref BGONE_AttackProfile_Base> m_eAttackProfileComponents;
	
	[Attribute("", UIWidgets.Object, desc: "Script Responsible For Physically Moving The Missile", category: "BGONE")]
	protected ref BGONE_MissileEngine_Base m_eMissileEngineComponent;
	
	protected int m_iArmingDistanceIndex = 0;
	protected int m_eAttackProfileComponentIndex = 0;
	protected Projectile m_eOwner;
	protected float m_fFlightTime;
	protected bool m_bGuidanceActive = false;
	protected ref BGONE_TargetData m_eCurrentTargetData;
	protected RplComponent m_RplComponent;
	BGONE_GuidedMissileLauncherComponent launcherComp;
	protected vector m_vLastTargetPosition = Vector(0,0,0);
	
	
	protected float m_fSaclosFixNextUpdateTime = 0;
	protected float m_fSaclosFixUpdateInterval = 50;
	
	// Enable for debug lines of target pos as missile flies.
	protected bool m_bDebugEnabled = false;
	protected ref array<ref Shape> m_aDbgCollisionShapes;
	
	protected vector m_vPreviousTargetPos;
	
	override void EOnInit(IEntity owner)
	{
		m_eOwner = Projectile.Cast(owner);
		m_RplComponent = RplComponent.Cast(m_eOwner.FindComponent(RplComponent));
		
		if(Replication.IsRunning() && Replication.IsClient() && m_RplComponent.Role() == RplRole.Authority)
			GetGame().GetCallqueue().CallLater(DeleteMissile, 100, false, owner); // Delete local copy of missile.
		
		if(!m_eOwner || !m_RplComponent || !launcherComp)
			return;

		m_aDbgCollisionShapes = new array<ref Shape>;
	}
	
	protected void DeleteMissile(IEntity owner)
	{
		m_RplComponent.DeleteRplEntity(owner, true);
	}
	
	void SetAttackAndFireModes(int attackModeIndex, int armingDistanceIndex)
	{
		m_eAttackProfileComponentIndex = attackModeIndex;
		m_iArmingDistanceIndex = armingDistanceIndex;
	}
	
	// Only ran on server
	array<ref Shape> onLaunched(BGONE_TargetData targetData, BGONE_GuidedMissileLauncherComponent launcher)
	{
		launcherComp = launcher;
		m_eCurrentTargetData = targetData;
		// Send initial data to seeker.
		SetAttackAndFireModes(m_eCurrentTargetData.attackProfileIndex, m_eCurrentTargetData.armingDistancesIndex);
		m_eSeekerTypeComponent.InitSeeker(m_eOwner, m_eCurrentTargetData);
		m_eAttackProfileComponents[m_eCurrentTargetData.attackProfileIndex].InitAttackMode(m_eOwner);
		
		m_fFlightTime = 0;
		
		// Can be replaced by setting simulate flag here?
		m_bGuidanceActive = true;
		
		return m_aDbgCollisionShapes;
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		
		// Checks can be replaced by setting simulate flag in onLaunched?
		if(m_RplComponent.Role() != RplRole.Authority)
			return;
		
		if(!m_bGuidanceActive)
			return;
		
		m_fFlightTime += timeSlice;

		// Pass Data To Seeker, Get Adjusted data back.
		m_eCurrentTargetData = m_eSeekerTypeComponent.ProcessFrame(m_eCurrentTargetData, m_fFlightTime);
		
		// SACLOS fix
		if(m_eSeekerTypeComponent.Type() == BGONE_SeekerType_SACLOS)
		{
			if(GetGame().GetWorld().GetWorldTime() > m_fSaclosFixNextUpdateTime)
			{
				m_fSaclosFixNextUpdateTime = GetGame().GetWorld().GetWorldTime() + m_fSaclosFixUpdateInterval;
				if(launcherComp)
					launcherComp.SaclosFix();
			}
		}
		
		// Use last target position if seeker lost track of target.
		if(m_eCurrentTargetData.targetPosition != Vector(0,0,0))
			m_vLastTargetPosition = m_eCurrentTargetData.targetPosition;
		else if(m_vLastTargetPosition != Vector(0,0,0))
			m_eCurrentTargetData.targetPosition = m_vLastTargetPosition;
		
		if(m_eCurrentTargetData.detonated > 0)
		{
			if(m_eCurrentTargetData.detonated == 1)
				Rpc(RpcAsk_Explode,false);
			if(m_eCurrentTargetData.detonated == 2)
				Rpc(RpcAsk_Explode,true);
			
			return;
		}
		
		// Pass Data To AttackMode, Get Adjusted Data Back.
		m_eCurrentTargetData = m_eAttackProfileComponents[m_eCurrentTargetData.attackProfileIndex].ProcessFrame(m_eCurrentTargetData, m_fFlightTime);
		
		if(m_bDebugEnabled)
		{
			if(!m_vPreviousTargetPos)
				m_vPreviousTargetPos = m_eCurrentTargetData.launchPos;
			
			Debug_DrawLineSimple(m_vPreviousTargetPos,m_eCurrentTargetData.targetPosition,m_aDbgCollisionShapes);
			m_vPreviousTargetPos = m_eCurrentTargetData.targetPosition;
			
			//Debug_DrawLineSimple(m_eCurrentTargetData.projectilePos,m_eCurrentTargetData.projectilePos + m_eCurrentTargetData.projectile.GetYawPitchRoll().AnglesToVector() * 10,m_aDbgCollisionShapes, ARGBF(1, 1, 0, 0));
		}
		
		// Pass Data To Engine, moving missile.
		if(m_fFlightTime > 0.5)
			m_eCurrentTargetData.detonated = m_eMissileEngineComponent.ProcessFrame(m_eOwner, m_eCurrentTargetData.targetPosition, m_fFlightTime, timeSlice);
		
		Rpc(RpcDo_UpdatePosVelAng, m_eOwner.GetOrigin(), m_eOwner.GetPhysics().GetVelocity(), m_eOwner.GetPhysics().GetVelocity().VectorToAngles());
	}
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	void RpcDo_UpdatePosVelAng(vector position, vector velocity, vector angles)
	{
		m_eOwner.SetYawPitchRoll(angles);
		m_eOwner.SetOrigin(position);
		m_eOwner.GetPhysics().SetVelocity(velocity);
	}

	void UpdateTurretAim(vector aimDir, vector aimPos)
	{
		Rpc(RpcDo_UpdateAimingDir,aimDir,aimPos);
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	void RpcDo_UpdateAimingDir(vector aimDir, vector aimPos)
	{
		BGONE_SeekerType_SACLOS seeker = BGONE_SeekerType_SACLOS.Cast(m_eSeekerTypeComponent);
		if(!seeker) return;
		
		seeker.UpdateAimingDirServer(aimDir,aimPos);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_Explode(bool down)
	{
		vector explodePos = m_eOwner.GetOrigin() + m_eOwner.GetPhysics().GetVelocity().Normalized() * 1.5;
		m_eOwner.SetOrigin(explodePos);
		if(down)
			m_eOwner.SetYawPitchRoll(Vector(-90,-90,-90));
		
		GetGame().GetCallqueue().CallLater(ExplodeWrapper, 1);
		Rpc(RpcDo_Explode, explodePos, down);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_Explode(vector pos, bool down)
	{
		m_eOwner.SetOrigin(pos);
		if(down)
			m_eOwner.SetYawPitchRoll(Vector(-90,-90,-90));
		
		ExplodeWrapper();
	}
	protected void ExplodeWrapper()
	{
		if(!m_eOwner)
			return;

		BaseTriggerComponent triggerComponent = BaseTriggerComponent.Cast(m_eOwner.FindComponent(BaseTriggerComponent));
		if (!triggerComponent)
			return;
		
		m_eOwner.Update();
		triggerComponent.SetLive();
		triggerComponent.OnUserTrigger(m_eOwner);
	}
	
	BGONE_AttackProfile_Base GetActiveAttackProfile()
	{
		return m_eAttackProfileComponents[m_eAttackProfileComponentIndex];
	}
	
	//Debug Methods
	private void Debug_DrawLineSimple(vector start, vector end, array<ref Shape> dbgShapes, int color = ARGBF(1, 1, 1, 1))
	{
		vector p[2];
		p[0] = start;
		p[1] = end;

		int shapeFlags = ShapeFlags.NOOUTLINE;
		Shape s = Shape.CreateLines(color, shapeFlags, p, 2);
		dbgShapes.Insert(s);	
	}
	private void Do_ClearDbgShapes()
	{
		m_aDbgCollisionShapes.Clear();
	}
	
	
	override void OnAddedToParent(IEntity child, IEntity parent)
	{
		// Make sure all required components are registered
		if(!m_eSeekerTypeComponent || m_eAttackProfileComponents.Count() == 0 || !m_eMissileEngineComponent)
		{
			Print("BGONE Guided Missile Component is missing required sub-components!", LogLevel.ERROR);
			return;
		}
		launcherComp = BGONE_GuidedMissileLauncherComponent.Cast(parent.FindComponent(BGONE_GuidedMissileLauncherComponent));
		if(!launcherComp)
			return;
		if(!m_eAttackProfileComponents[m_eAttackProfileComponentIndex])
			return;
		if(!m_eSeekerTypeComponent)
			return;
		launcherComp.SetAvailableAttackProfiles(m_eAttackProfileComponents);
		launcherComp.SetAvailableArmingDistances(m_eSeekerTypeComponent.GetAvailableArmingDistances());
	}
	override void OnPostInit(IEntity owner)
	{
		SetEventMask( owner, EntityEvent.INIT | EntityEvent.SIMULATE);
	}
};
