fileIn "constants.ms"

persistent global VINE_logID

global VINE_triggersHidden = true
global VINE_spatialIndexHidden = true

plugin Geometry VINE_Trigger
name:"Trigger"
classID:#(TAG_level_class, TAG_level_trigger)
category:"VINE" 
extends:Box
autoPromoteDelegateProps:true
replaceUi:true
version:1
invisible:false
(
	local self
	parameters pBlock rollout:params (
		script type:#string animatable:false ui:scriptTXT default:""
		args type:#string animatable:false ui:argsTXT default:""
		selfId type:#integer animatable:false default:-1
	)
	
	rollout params "Trigger parameters"
	(
		editText scriptTXT "Script name"
		editText argsTXT "Script args"
		
		spinner lengthSPN "Length:" range:[0,1000,0] type:#float
		spinner widthSPN "Width:" range:[0,1000,0] type:#float
		spinner heightSPN "Height:" range:[0,1000,0] type:#float
		
		on params open do (
			lengthSPN.value = delegate.length
			widthSPN.value = delegate.width
			heightSPN.value = delegate.height
		)
		
		on lengthSPN changed val do(
			delegate.length = val
			)
		on widthSPN changed val do(
			delegate.width = val
			)
		on heightSPN changed val do(
			delegate.height = val
			)
	)
	
	on attachedToNode n do (
		self = n
		self.material = VINE_TriggerMaterial
		self.wirecolor = VINE_TriggerMaterial.diffuse
		
		if selfID == -1 then (
			if VINE_logID == undefined then VINE_logID = 0
			VINE_logID += 1
			selfID = VINE_logID
		)
	)
	
	on load do
		if self==undefined then
		for obj in $*
			where
				classOf obj == VINE_Trigger
			do if obj.selfID == selfID then (
				self = obj
				break
			)
)

plugin Geometry VINE_CheckPointTrigger
name:"CP Trigger"
classID:#(TAG_level_class, TAG_level_checkpointTrigger)
category:"VINE" 
extends:Box
autoPromoteDelegateProps:true
replaceUi:true
version:1
invisible:true
(
	local self
	parameters pBlock (
		selfId type:#integer animatable:false default:-1
	)
	
	rollout cpTriggerParams_RO "Parameters"
	(
		spinner lengthSPN "Length:" range:[0,1000,0] type:#float
		spinner widthSPN "Width:" range:[0,1000,0] type:#float
		spinner heightSPN "Height:" range:[0,1000,0] type:#float
		
		on cpTriggerParams_RO open do (
			lengthSPN.value = delegate.length
			widthSPN.value = delegate.width
			heightSPN.value = delegate.height
		)
		
		on lengthSPN changed val do(
			delegate.length = val
			)
		on widthSPN changed val do(
			delegate.width = val
			)
		on heightSPN changed val do(
			delegate.height = val
			)
	)
	
	rollout checkpointTriggerChild_RO "Spawn point"
	(
		edittext spawnTXT "" enabled:false
		button refreshBTN "Refresh" pos:[88,48] width:64 height:21
		button selectSpawnBTN "Select" pos:[8,48] width:64 height:21
		
		fn getSpawns = (
			if not isGone self
				then return \
					for a in self.children 
						where ((classof a) as string) == "VINE_Checkpoint" 
						collect a
				else return #()
		)
		
		fn showSpawn = (
			local spawns = getSpawns()
			if spawns.count <= 0 then spawnTXT.text = "!~ No VINE_Checkpoint children"
			else if spawns.count > 1 then spawnTXT.text = "!~ Multiple children"
			else spawnTXT.text = "$"+(spawns[1].name)
		)
		
		on checkpointTriggerChild_RO open do
			showSpawn()
		on refreshBTN pressed do
			showSpawn()
		on selectSpawnBTN pressed do (
			local spawns = getSpawns()
			if spawns.count > 0 then select spawns
		)
	)
	
	on attachedToNode n do (
		self = n
		self.material = VINE_CheckPointMaterial
		self.wirecolor = VINE_CheckPointMaterial.diffuse
		
		if selfID == -1 then (
			if VINE_logID == undefined then VINE_logID = 0
			VINE_logID += 1
			selfID = VINE_logID
		)
	)
	
	on load do
		if self==undefined then
		for obj in $*
			where
				classOf obj == VINE_CheckPointTrigger
			do (
				if obj.selfID == selfID then (
				self = obj
				break
			)
		)
)

plugin Geometry VINE_SpatialIndex
name:"Spatial Index"
classID:#(TAG_level_class, TAG_level_spatialIndex)
category:"VINE" 
extends:Box
autoPromoteDelegateProps:true
replaceUi:true
version:1
invisible:false
(
	local self
	parameters pBlock rollout:params (
		index type:#integer animatable:false ui:indexSPN default:1
		selfId type:#integer animatable:false default:-1
	)
	
	rollout params "Spatial Index"
	(
		spinner indexSPN "Index:" type:#integer range:[0,255,0]
		
		spinner lengthSPN "Length:" range:[0,1000,0] type:#float
		spinner widthSPN "Width:" range:[0,1000,0] type:#float
		spinner heightSPN "Height:" range:[0,1000,0] type:#float
		
		on params open do (
			lengthSPN.value = delegate.length
			widthSPN.value = delegate.width
			heightSPN.value = delegate.height
		)
		
		on lengthSPN changed val do(
			delegate.length = val
			)
		on widthSPN changed val do(
			delegate.width = val
			)
		on heightSPN changed val do(
			delegate.height = val
			)
	)
	
	on attachedToNode n do (
		self = n
		self.material = VINE_SpatialIndexMaterial
		self.wirecolor = VINE_SpatialIndexMaterial.diffuse
		
		if selfID == -1 then (
			if VINE_logID == undefined then VINE_logID = 0
			VINE_logID += 1
			selfID = VINE_logID
		)
		local indices = for a in $* where classof a == VINE_SpatialIndex collect a.index
		if indices.count > 0 then index = (amax indices)+1
	)
	
	on load do
		if self==undefined then
		for obj in $*
			where
				classOf obj == VINE_SpatialIndex
			do if obj.selfID == selfID then (
				self = obj
				break
			)
)

plugin Geometry VINE_Destructible
name:"Destructible"
classID:#(TAG_level_class, TAG_level_destructible)
category:"VINE" 
extends:Box
autoPromoteDelegateProps:true
replaceUi:true
version:1
invisible:false
(
	rollout params "Destructible wall"
	(
		spinner lengthSPN "Length:" range:[0,1000,0] type:#float
		spinner widthSPN "Width:" range:[0,1000,0] type:#float
		spinner heightSPN "Height:" range:[0,1000,0] type:#float
		
		on params open do (
			lengthSPN.value = delegate.length
			widthSPN.value = delegate.width
			heightSPN.value = delegate.height
		)
		
		on lengthSPN changed val do(
			delegate.length = val
			)
		on widthSPN changed val do(
			delegate.width = val
			)
		on heightSPN changed val do(
			delegate.height = val
			)
	)
	
	on attachedToNode n do (
		n.material = VINE_DestructibleMaterial
		n.wirecolor = VINE_DestructibleMaterial.diffuse
	)
)

plugin Geometry VINE_SmokeHazard
name:"Smoke panel"
classID:#(TAG_level_class, TAG_level_SmokeHazard)
category:"VINE" 
extends:Box
autoPromoteDelegateProps:true
replaceUi:true
version:1
invisible:false
(
	
	rollout hazardRO "Smoke hazard" (
		spinner indexTXT "Index:" type:#integer default:0
		spinner subindexTXT "Subindex:" type:#integer default:0
	)
	parameters objectParams rollout:hazardRO (
		index type:#integer animatable:false ui:indexTXT default:0
		subindex type:#integer animatable:false ui:subindexTXT default:0
	)
	
	rollout params "Box"
	(
		spinner lengthSPN "Length:" range:[0,1000,0] type:#float
		spinner widthSPN "Width:" range:[0,1000,0] type:#float
		spinner heightSPN "Height:" range:[0,1000,0] type:#float
		
		on params open do (
			lengthSPN.value = delegate.length
			widthSPN.value = delegate.width
			heightSPN.value = delegate.height
		)
		
		on lengthSPN changed val do(
			delegate.length = val
			)
		on widthSPN changed val do(
			delegate.width = val
			)
		on heightSPN changed val do(
			delegate.height = val
			)
	)
	
	on attachedToNode n do (
		n.material = VINE_SmokeBoxMaterial
		n.wirecolor = VINE_SmokeBoxMaterial.diffuse
	)
)

plugin Geometry VINE_WildcardBox
name:"Wildcard"
classID:#(TAG_level_class, TAG_level_WildcardBox)
category:"VINE" 
extends:Box
autoPromoteDelegateProps:true
replaceUi:true
version:1
invisible:false
(
	
	rollout wildcardRO "Wildcard" (
		edittext tag1TXT "Tag 1:"
		edittext tag2TXT "Tag 2:"
		edittext tag3TXT "Tag 3:"
	)
	parameters objectParams rollout:wildcardRO (
		tag1 type:#string animatable:false ui:tag1TXT default:""
		tag2 type:#string animatable:false ui:tag2TXT default:""
		tag3 type:#string animatable:false ui:tag3TXT default:""
	)
	
	rollout params "Box"
	(
		spinner lengthSPN "Length:" range:[0,1000,0] type:#float
		spinner widthSPN "Width:" range:[0,1000,0] type:#float
		spinner heightSPN "Height:" range:[0,1000,0] type:#float
		
		on params open do (
			lengthSPN.value = delegate.length
			widthSPN.value = delegate.width
			heightSPN.value = delegate.height
		)
		
		on lengthSPN changed val do(
			delegate.length = val
			)
		on widthSPN changed val do(
			delegate.width = val
			)
		on heightSPN changed val do(
			delegate.height = val
			)
	)
	
	on attachedToNode n do (
		n.material = VINE_WildCardBoxMaterial
		n.wirecolor = VINE_WildCardBoxMaterial.diffuse
	)
)

plugin Geometry VINE_WhiteBox
name:"WhiteBox"
classID:#(TAG_level_class, TAG_level_whitebox)
category:"VINE" 
extends:Box
autoPromoteDelegateProps:true
version:1
invisible:false
(
	on attachedToNode n do (
		n.wirecolor = white
	)
)

fn LOAD_MACRO_VINE_ToggleTrigger = (
	macroScript MACRO_VINE_ToggleTrigger
		category:"VINEDETTA"
		toolTip: "Toggle trigger display"
		icon:#("toggleTriggers", 1)
	(
		VINE_triggersHidden = not VINE_triggersHidden
		for obj in $*
			where classof obj == VINE_Trigger \
			or classof obj == VINE_CheckPointTrigger \
			or classof obj == VINE_SmokeHazard \
			or classof obj == VINE_Destructible do (
			obj.isHidden = VINE_triggersHidden
		)
	)
	OK
)
LOAD_MACRO_VINE_ToggleTrigger()

fn LOAD_MACRO_VINE_ToggleSpatialIndex = (
	macroScript MACRO_VINE_ToggleSpatialIndex
		category:"VINEDETTA"
		toolTip: "Toggle spatial indexing display"
		icon:#("toggleSpatial", 1)
	(
		VINE_spatialIndexHidden = not VINE_spatialIndexHidden
		for obj in $* 
			where classof obj == VINE_SpatialIndex \
			or classof obj == VINE_WildcardBox do (
			obj.isHidden = VINE_spatialIndexHidden
		)
	)
	OK
)
LOAD_MACRO_VINE_ToggleTrigger()


