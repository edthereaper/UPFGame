clearListener()

fileIn "tools.ms"

--Export bounding boxes only!

fn exportWhiteBoxes = (
	local data_path = "C:\\Users\\Eddy\\Documents\\GameUPF\\VineDetta\\engine\\engine\\data\\"
	--local data_path = "c:\\mcv13-APA-data\\vinedetta\\engine\\engine\\data\\"
	--local data_path = "D:\\bitBucket\\Vinedetta\\engine\\engine\\data\\"
	local filename = data_path + "whitebox\\"
	filename += "export.whitebox"
	local fs = fopen filename "wb"
	if fs == undefined then throw ( "Can't create whitebox output file " + filename )
	
	local boxTag 				= 0x12345601
	local trampolineTag 		= 0x12345602
	local cannonTag 			= 0x12345603
	local lianaTag 			= 0x12345604
	local creepTag 			= 0x12345605
	local meshTag 			= 0x12345606
	local destructibleTag 	= 0x12345607
	local checkpointTag		= 0x12345608
	local pickupTag		    = 0x12345609
	local enemyTag			= 0x1234560A
	local wpTag				= 0x1234560B
	
	local levelTag				= 0x12345610
	local eofTag 				    = 0x123456FF
	local boxColliderTag      = 0x21345FF2
	local meshColliderTag   = 0x23154FF2
	
	local LianaBaseTag    = 0x12934A0C

	local startPosition = [0,0,0,0]
	local startRotation = [0,0,0,1]
	local finishMin  = [0,0,0,0]
	local finishMax =[-1.f,-1.f,-1.f,-1.f]
	
	local trampolineColor 			= (color 8 61 138)
	local cannonColor 				= (color 135 59 8)
	local lianaColor 					= (color 0 83 0)
	local creepColor 					= (color 61 135 6)
	local destructibleColor 			= (color 225 143 87)
	local checkpointColor 			= (color 148 177 27)
	local finishColor 					= (color 135 6 6)

	local lifePickupColor				= (color 108 8 136)
	local energyPickupColor		= (color 8 61 138)
	local invenciblePickupColor	= (color 177 148 27)

	local spawnColor 				= (color 6 135 6)
	local meleeEnemyColor			= (color 135 6 6)
	local flareEnemyColor			= (color 177 148 27)
	local sniperEnemyColor			= (color 8 8 136)
	
	local boxColliderColor			= (color 6 135 58)
	
	local LianaBaseColor		    	= (color 143 225 87)
	
	local meshColliderColor			= (color 57 8 136)
	
	local wpList = #()
	
	for obj in $wp* do (
		if classof obj == line then
			append wpList obj
	)
	
	for obj in Geometry do (
		
		if classof obj ==Editable_poly then (
			
			local pos = max2mcv obj.pos
			local bbmin = max2mcv obj.min
			local bbmax = max2mcv obj.max
			minmax &bbmin &bbmax
			local mesh = getUserProp obj "mesh"
			if mesh != undefined then (
				
				local trimmedName = substring (mesh)1 31
				local meshColor = obj.wirecolor
				local rot = max2mcvQuat obj.rotation
				
				if obj.wirecolor == meshColliderColor then(
					format "Entro al mesh collider: %\n" trimmedName
					WriteLong fs meshColliderTag #unsigned --begin meshCollider tag
				)else(
					WriteLong fs meshTag #unsigned -- begin common mesh tag
				)
				
				WriteFloat fs pos.x
				WriteFloat fs pos.y
				WriteFloat fs pos.z
				WriteFloat fs rot.x
				WriteFloat fs rot.y
				WriteFloat fs rot.z
				WriteFloat fs rot.w
				WriteFloat fs bbmin.x
				WriteFloat fs bbmin.y
				WriteFloat fs bbmin.z
				WriteFloat fs bbmax.x
				WriteFloat fs bbmax.y
				WriteFloat fs bbmax.z
				WriteByte fs meshColor.r
				WriteByte fs meshColor.g
				WriteByte fs meshColor.b
				WriteByte fs meshColor.a
				WriteString fs trimmedName
					
				for i = (trimmedName.count+1) to 31 do (
					WriteByte fs 0x00
				)
				
				if obj.wirecolor == meshColliderColor then(
					format "salio del mesh collider: %\n" trimmedName
					WriteLong fs meshColliderTag #unsigned --end meshCollider tag
				)else(
					WriteLong fs meshTag #unsigned --end common mesh tag
				)
				
				
				--format "Exporting object % as mesh\n" obj.name
				--format "\tpos=% min=% max=% name=%\n" pos bbmin bbmax meshName
			) else (
				format "Object % won't be exported (no property "mesh")\n" obj.name
			)
		) else if classof obj == Box then (
			
			--- if every mesh its a Box Shape
			
			if obj.wirecolor == trampolineColor then (
				local pos = max2mcv obj.pos
				local rot = max2mcvQuat obj.rotation
				
				WriteLong fs trampolineTag #unsigned
				WriteFloat fs pos.x
				WriteFloat fs pos.y
				WriteFloat fs pos.z
				WriteFloat fs rot.x
				WriteFloat fs rot.y
				WriteFloat fs rot.z
				WriteFloat fs rot.w
				WriteLong fs trampolineTag #unsigned
				--format "Exporting object % as trampoline\n" obj.name
				--format "\tpos=% rot=%\n" pos rot
				
			)else if obj.wirecolor == boxColliderColor then (
				
				local pos = max2mcv obj.pos
				local rot = max2mcvQuat obj.rotation
				
				local height = obj.height
				local len = obj.length
				local width = obj.width
				
				WriteLong fs boxColliderTag #unsigned
				WriteFloat fs pos.x
				WriteFloat fs pos.y
				WriteFloat fs pos.z
				WriteFloat fs rot.x
				WriteFloat fs rot.y
				WriteFloat fs rot.z
				WriteFloat fs rot.w
				WriteFloat fs height
				WriteFloat fs len
				WriteFloat fs width
				WriteLong fs boxColliderTag #unsigned
				--format "Exporting object % as trampoline\n" obj.name
				--format "\tpos=% rot=%\n" pos rot
				
			)else if obj.wirecolor == LianaBaseColor then (
				
				local pos = max2mcv obj.pos
				local rot = max2mcvQuat obj.rotation
				
				local height = obj.height
				local len = obj.length
				local width = obj.width
				
				WriteLong fs LianaBaseTag #unsigned
				WriteFloat fs pos.x
				WriteFloat fs pos.y
				WriteFloat fs pos.z
				WriteFloat fs rot.x
				WriteFloat fs rot.y
				WriteFloat fs rot.z
				WriteFloat fs rot.w
				WriteLong fs LianaBaseTag #unsigned
				format "Exporting object: % as Liana\n" obj.name
				format "\tpos=% rot=%\n" pos rot
				
			) else if obj.wirecolor == cannonColor then (
				local pos = max2mcv obj.pos
				local rot = max2mcvQuat obj.rotation
				
				-- fov as radians
				local fovH = getUserProp obj "fovH"
				if (fovH == undefined) then (fovH = 30.f)
				else (fovH = fovH as float)
				local fovV= getUserProp obj "fovV"
				if (fovV == undefined) then (fovV = 30.f)
				else (fovV = fovV as float)
				fovH *= pi/180
				fovV *= pi/180			
				
				WriteLong fs cannonTag #unsigned
				WriteFloat fs pos.x
				WriteFloat fs pos.y
				WriteFloat fs pos.z
				WriteFloat fs rot.x
				WriteFloat fs rot.y
				WriteFloat fs rot.z
				WriteFloat fs rot.w
				WriteFloat fs fovH
				WriteFloat fs fovV
				WriteLong fs cannonTag #unsigned
				--format "Exporting object % as cannon\n" obj.name
				--format "\tpos=% rot=% fovH=% fovV=%\n" pos rot fovH fovV
				
			) else if obj.wirecolor == lianaColor then (
				local pos = max2mcv obj.pos
				local h = abs obj.height
				WriteLong fs lianaTag #unsigned
				WriteFloat fs pos.x
				WriteFloat fs pos.y
				WriteFloat fs pos.z
				WriteFloat fs h
				WriteLong fs lianaTag #unsigned
				--format "Exporting object % as liana\n" obj.name
				--format "\tpos=%\n" pos
			) else if  obj.wirecolor == creepColor then (
				local pos = max2mcv obj.pos
				local height = obj.height
				local len = obj.length
				local width = obj.width
				
				WriteLong fs creepTag #unsigned
				WriteFloat fs pos.x
				WriteFloat fs pos.y
				WriteFloat fs pos.z
				WriteFloat fs height
				WriteFloat fs len
				WriteFloat fs width
				WriteLong fs creepTag #unsigned
				--format "Exporting object % as creep\n" obj.name
				--format "\theight=% width=%\n"height width
			) else if  obj.wirecolor == destructibleColor then (
				local bbmin = max2mcv obj.min
				local bbmax = max2mcv obj.max
				minmax &bbmin &bbmax
				
				WriteLong fs destructibleTag #unsigned
				WriteFloat fs bbmin.x
				WriteFloat fs bbmin.y
				WriteFloat fs bbmin.z
				WriteFloat fs bbmax.x
				WriteFloat fs bbmax.y
				WriteFloat fs bbmax.z
				WriteLong fs destructibleTag #unsigned
				--format "Exporting object % as destructible box\n" obj.name
				--format "\tmin=% max=%\n" bbmin bbmax
			) else if  obj.wirecolor == finishColor then (
				finishMin = max2mcv obj.min
				finishMax = max2mcv obj.max
				minmax &finishMin &finishMax
			) else if  obj.wirecolor == checkpointColor then (
				local bbmin = max2mcv obj.min
				local bbmax = max2mcv obj.max
				minmax &bbmin &bbmax
			
				local pos = max2mcv obj.pos
				
				WriteLong fs checkpointTag #unsigned
				WriteFloat fs bbmin.x
				WriteFloat fs bbmin.y
				WriteFloat fs bbmin.z
				WriteFloat fs bbmax.x
				WriteFloat fs bbmax.y
				WriteFloat fs bbmax.z
				WriteFloat fs pos.x
				WriteFloat fs pos.y
				WriteFloat fs pos.z
				WriteLong fs checkpointTag #unsigned
			) else (
				local bbmin = max2mcv obj.min
				local bbmax = max2mcv obj.max
				minmax &bbmin &bbmax
				local boxColor = obj.wirecolor
				
				WriteLong fs boxTag #unsigned
				WriteFloat fs bbmin.x
				WriteFloat fs bbmin.y
				WriteFloat fs bbmin.z
				WriteFloat fs bbmax.x
				WriteFloat fs bbmax.y
				WriteFloat fs bbmax.z
				WriteByte fs boxColor.r
				WriteByte fs boxColor.g
				WriteByte fs boxColor.b
				WriteByte fs boxColor.a
				WriteLong fs boxTag #unsigned
				--format "Exporting object % as box\n" obj.name
				--format "\tmin=% max=% color=%\n" bbmin bbmax boxColor
			)
		) else if classof obj == Sphere then (
			local type = undefined
			if obj.wirecolor == lifePickupColor then (
				type = 1
			) else if obj.wirecolor == energyPickupColor then (
				type = 2
			) else if obj.wirecolor == invenciblePickupColor then (
				type = 3
			)
			
			if type != undefined then (
				local pos = max2mcv obj.pos
				WriteLong fs pickupTag #unsigned
				WriteFloat fs pos.x
				WriteFloat fs pos.y
				WriteFloat fs pos.z
				WriteLong fs type #unsigned
				WriteLong fs pickupTag #unsigned
			) else (
				format "Object % won't be exported (no pickup of color code %)\n" obj.name obj.wirecolor
			)
		) else if classof obj == Pyramid then (
			if  obj.wirecolor == spawnColor then (
				startPosition = max2mcv obj.pos
				startRotation = max2mcvQuat obj.rotation
			) else (
				local type = undefined
				if obj.wirecolor == meleeEnemyColor then (
					type = 1
				) else if obj.wirecolor == flareEnemyColor then (
					type = 2
				) else if obj.wirecolor == sniperEnemyColor then (
					type = 3
				)
				if type != undefined then (
					local pos = max2mcv obj.pos
					local rot = max2mcvQuat obj.rotation
					local wpname = getUserProp obj "wp"
					local wp = undefined
					
					if wpname != unsigned then (
						for i=1 to wpList.count do (
							if wpList[i].name == wpname then (
								wp = i
								exit with OK
							)
						)
					) else (
						wp = 0
					)
					
					if wp != undefined then (
						WriteLong fs enemyTag #unsigned
						WriteFloat fs pos.x
						WriteFloat fs pos.y
						WriteFloat fs pos.z
						WriteFloat fs rot.x
						WriteFloat fs rot.y
						WriteFloat fs rot.z
						WriteFloat fs rot.w
						WriteLong fs wp #unsigned
						WriteLong fs type #unsigned
						WriteLong fs enemyTag #unsigned
					) else (
						format "Object % won't be exported (no waypoint of name %)\n" obj.name wpname
					)
				) else (
					format "Object % won't be exported (no enemy of color code %)\n" obj.name obj.wirecolor
				)
			)
		) else (
			format "Object % won't be exported\n" obj.name
		)
	)
	
	for wp in wpList do (
		local ns = numsplines wp
		local nk = numknots wp
		WriteLong fs wpTag #unsigned
		WriteLong fs (nk*ns) #unsigned
		for s=1 to ns do (
			for k=1 to nk do (
				local p = max2mcv (getKnotPoint wp s k)
				WriteFloat fs p.x
				WriteFloat fs p.y
				WriteFloat fs p.z
			)
		)
		WriteLong fs wpTag #unsigned
	)
	
	WriteLong fs levelTag #unsigned
	WriteFloat fs startPosition.x
	WriteFloat fs startPosition.y
	WriteFloat fs startPosition.z
	WriteFloat fs startRotation.x
	WriteFloat fs startRotation.y
	WriteFloat fs startRotation.z
	WriteFloat fs startRotation.w
	WriteFloat fs finishMin.x
	WriteFloat fs finishMin.y
	WriteFloat fs finishMin.z
	WriteFloat fs finishMax.x
	WriteFloat fs finishMax.y
	WriteFloat fs finishMax.z
	WriteLong fs levelTag #unsigned
	WriteLong fs eofTag #unsigned
	OK
)

exportWhiteBoxes()
gc()