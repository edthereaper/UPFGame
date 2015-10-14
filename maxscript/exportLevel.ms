fileIn "tools.ms"
fileIn "export_mesh.ms"
fileIn "levelPieces.ms"
fileIn "levelObjects.ms"
fileIn "levelLogic.ms"
fileIn "export_camera.ms"
fileIn "exportProperties.ms"

struct VINE_exportLevel (
	sessionCols = #(),
	piecesExported = 0,
	enemiesExported = 0,
	objectsExported = 0,
	triggersExported = 0,
	spatialExported = 0,
	whiteboxesExported = 0,
	destructiblesExported = 0,
	smokesExported = 0,
	wildcardsExported = 0,
	wildcardboxesExported = 0,
	collisionsExported = 0,
	cameraExported = 0,
	
	fn getDataPath = (
		local ret = getINISetting VINE_settingsFile "settings" "data_path"
		if ret == "" then throw("No data path set.")
		return ret
	),
	
	fn isSpecialCollisionable col = (
		if col.parent == undefined then return true
		else (
			for obj in col.parent where classof obj == VINE_Piece do (
				return isTransformablePiece obj or isSpecialPiece obj
			)
			return false
		)
	),
	
	fn isSpecialCollision col = (
		if getUserProp col "special" != undefined then return true
		if getUserProp col "special2" != undefined then return true
			return false
	),
	
	fn allCollisions transformables:true = (
		local whole = TriMesh()
		for col in $* where col.name == "COLLISION" do 
			if (transformables or not isSpecialCollisionable col) and not isSpecialCollision col
				then (
					local col2 = copy col
					meshop.attach whole col2
				)
		return whole
	),
	
	fn allSpecialCollisions = (
		return for a in $* where a.name=="COLLISION" and isSpecialCollision a collect a
	),
	fn allSmokeHazards = (
		return for a in $* where classof a == VINE_SmokeHazard collect a
	),
	fn allWildcards = (
		return for a in $* where classof a == VINE_Wildcard collect a
	),
	fn allWildcardBoxes = (
		return for a in $* where classof a == VINE_WildcardBox collect a
	),
	fn allDestructibles = (
		return for a in $* where classof a == VINE_Destructible collect a
	),
	fn allWhiteBoxes = (
		return for a in $* where classof a == VINE_WhiteBox collect a
	),
	
	fn allPieces = (
		return for a in $* where classof a == VINE_Piece collect a
	),
	
	fn allSpatial = (
		return for a in $* where classof a == VINE_SpatialIndex  collect a
	),
	
	fn allTriggers = (
		return for a in $* where classof a == VINE_Trigger collect a
	),
	
	fn allCamera = (
		return for a in $* where classof a == VINE_Camera collect a
	),
	
	fn allEnemies = (
		local ans = #()
		join ans (for a in $* where classof a == VINE_Melee collect a)
		join ans (for a in $* where classof a == VINE_Flare collect a)
		join ans (for a in $* where classof a == VINE_Sniper collect a)
		return ans
	),
	
	fn allObjects = (
		local ans = #()
		join ans (for a in $* where classof a == VINE_Trampoline collect a)
		join ans (for a in $* where classof a == VINE_Cannon collect a)
		join ans (for a in $* where classof a == VINE_Liana collect a)
		join ans (for a in $* where classof a == VINE_Creep collect a)
		join ans (for a in $* where classof a == VINE_Checkpoint collect a)
		join ans (for a in $* where classof a == VINE_Pickup collect a)
		return ans
	),
	
	
	fn spatialIndexOf obj = (
		local all = allSpatial() 
		for spatial in all do (
			if 
				obj.position.x <= spatial.max.x and obj.position.x >= spatial.min.x and \
				obj.position.y <= spatial.max.y and obj.position.y >= spatial.min.y and \
				obj.position.z <= spatial.max.z and obj.position.z >= spatial.min.z
			then (return spatial.index; break)
		)
		return -1
	),
	
	fn writeTransform obj fs rot:true sca:false = (
		local q = max2mcvQuat obj.rotation
		local pos = max2mcv obj.pos
		local scal = max2mcv obj.scale
		format " pos=\"% % %\"" pos.x pos.y pos.z to:fs
		if rot then
			format " rot=\"% % % %\"" q.x q.y q.z q.w to:fs
		if sca then
			format " sca=\"% % % %\"" scal.x scal.y scal.z to:fs
		--if isAnimated obj == true then
		--	format " animated=true animatedFile=\"%\"" obj.name to:fs 
		--else
		--	format " animtad=false"
	),
	
	fn writeSpatialIndex obj fs = (
		local index = spatialIndexOf obj
		format " spatialIndex=\"%\"" index to:fs
	),

	fn exportSpatial fs quick:false = (
		local all = allSpatial()
		format "\t<spatials count=\"%\">\n" all.count to:fs
		for spatial in all do (
			format "\t\t<spatial"to:fs
			format " spatialIndex=\"%\"" spatial.index to:fs
			writeTransform spatial fs rot:false
			VINE_writeUserProps spatial fs:fs
			format " width=\"%\" length=\"%\" height=\"%\"" \
				spatial.width spatial.length spatial.height to:fs
			spatialExported += 1
			format "/>\n"to:fs
		)
		format "\t</spatials>\n" to:fs
	),
	
	fn exportWildcards fs quick:false = (
		local all = allWildcards()
		format "\t<wildcards count=\"%\">\n" all.count to:fs
		for w in all do (
			format "\t\t<wildcard"to:fs
			writeTransform w fs
			writeSpatialIndex w fs
			VINE_writeUserProps w fs:fs
			if w.tag1 != "" then format " tag1=\"%\"" w.tag1 to:fs
			if w.tag2 != "" then format " tag2=\"%\"" w.tag2 to:fs
			if w.tag3 != "" then format " tag3=\"%\"" w.tag3 to:fs
			wildcardsExported += 1
			format "/>\n"to:fs
		)
		format "\t</wildcards>\n" to:fs
	),
	
	fn exportDestructibles fs quick:false = (
		local all = allDestructibles()
		format "\t<destructibles count=\"%\">\n" all.count to:fs
		for w in all do (
			format "\t\t<destructible"to:fs
			writeTransform w fs
			writeSpatialIndex w fs
			VINE_writeUserProps w fs:fs
			format " width=\"%\" length=\"%\" height=\"%\"" \
				w.width w.length w.height to:fs
			destructiblesExported += 1
			format "/>\n"to:fs
		)
		format "\t</destructibles>\n" to:fs
	),
	
	fn exportSmokeHazards fs quick:false = (
		local all = allSmokeHazards()
		format "\t<smokes count=\"%\">\n" all.count to:fs
		for w in all do (
			format "\t\t<smoke"to:fs
			writeTransform w fs
			writeSpatialIndex w fs
			VINE_writeUserProps w fs:fs
			local c = w.wirecolor
			format " width=\"%\" length=\"%\" height=\"%\" index=\"%\" subindex=\"%\" />\n" \
				w.width w.length w.height w.index w.subindex to:fs
			smokesExported += 1
		)
		format "\t</smokes>\n" to:fs
	),
	fn exportWhiteBoxes fs quick:false = (
		local all = allWhiteBoxes()
		format "\t<whiteboxes count=\"%\">\n" all.count to:fs
		for w in all do (
			format "\t\t<whitebox"to:fs
			writeTransform w fs
			writeSpatialIndex w fs
			VINE_writeUserProps w fs:fs
			local c = w.wirecolor
			format " width=\"%\" length=\"%\" height=\"%\" color=\"% % %\" />\n" \
				w.width w.length w.height c.r c.g c.b to:fs
			whiteboxesExported += 1
		)
		format "\t</whiteboxes>\n" to:fs
	),
	
	fn exportWildcardBoxes fs quick:false = (
		local all = allWildcardBoxes()
		format "\t<wildcardBoxes count=\"%\">\n" all.count to:fs
		for w in all do (
			format "\t\t<wildcardBox"to:fs
			writeTransform w fs
			writeSpatialIndex w fs
			VINE_writeUserProps w fs:fs
			if w.tag1 != "" then format " tag1=\"%\"" w.tag1 to:fs
			if w.tag2 != "" then format " tag2=\"%\"" w.tag2 to:fs
			if w.tag3 != "" then format " tag3=\"%\"" w.tag3 to:fs
			format " width=\"%\" length=\"%\" height=\"%\" />\n" \
				w.width w.length w.height to:fs
			wildcardboxesExported += 1
		)
		format "\t</wildcardBoxes>\n" to:fs
	),
	
	fn exportTriggers fs quick:false = (
		local all = allTriggers()
		format "\t<triggers count=\"%\">\n" all.count to:fs
		for trigger in all do (
			format "\t\t<trigger"to:fs
			writeTransform trigger fs rot:false sca:false
			writeSpatialIndex trigger fs
			VINE_writeUserProps trigger fs:fs
			format " width=\"%\" length=\"%\" height=\"%\"" \
				trigger.width trigger.length trigger.height to:fs
			format " script=\"%\" args=\"%\"" trigger.script trigger.args to:fs
			triggersExported += 1
			format "/>\n"to:fs
		)
		format "\t</triggers>\n" to:fs
	),
	
	fn exportEnemies fs quick:false = (
		local all = allEnemies()
		format "\t<enemies count=\"%\">\n" all.count to:fs
		for enemy in all do (
			format "\t\t<enemy"to:fs
			writeTransform enemy fs
			writeSpatialIndex enemy fs
			local super = enemy.delegate
			VINE_writeUserProps enemy fs:fs
			format ">\n" to:fs
			
			case classof enemy of (
				VINE_Melee: format "\t\t\t<melee />\n" to:fs
				VINE_Flare: format "\t\t\t<flare />\n" to:fs
				VINE_Sniper: format "\t\t\t<sniper />\n" to:fs
			)
			format "\t\t</enemy>\n" to:fs
			enemiesExported += 1
		)
		format "\t</enemies>\n" to:fs
	),
	
	fn exportObjects fs quick:false = (
		local all = allObjects() 
		format "\t<objects count=\"%\">\n" all.count to:fs
		for obj in all do (
			format "\t\t<object"to:fs
			local transformObj = obj -- who holds the transform I want
			local super = obj.delegate
			
			-- creeps are indirect objects. The actual transform is the plane's
			if classof obj == VINE_Creep then (
				local parent = super.thisNode.parent
				if parent != undefined and classof parent == Plane then (
					transformObj = parent
				)
			)
			writeTransform transformObj fs
			writeSpatialIndex transformObj fs
			format " transformed=\"%\"" super.transformed to:fs
			VINE_writeUserProps obj fs:fs
			format ">\n" to:fs
			
			case classof obj of (
				VINE_CheckPoint: (
					format "\t\t\t<checkPoint isSpawn=\"%\" order=\"%\"" obj.isSpawn obj.order to:fs
					local parent = super.thisNode.parent
					if not isGone parent and classof parent == VINE_CheckPointTrigger then (
						format " trigger=\"yes\"" to:fs
						writeTransform parent fs rot:false
						writeSpatialIndex parent fs
						VINE_writeUserProps parent fs:fs
						format " width=\"%\" length=\"%\" height=\"%\" />\n" \
							parent.width parent.length parent.height to:fs
					) else (
						format " trigger=\"no\" />\n" to:fs
					)
				)
				VINE_Trampoline: format "\t\t\t<trampoline />\n" to:fs
				VINE_Cannon: format "\t\t\t<cannon fovH=\"%\" fovV=\"%\" impulse=\"%\" />\n" \
					obj.fovH obj.fovV obj.impulse to:fs
				VINE_Liana: format "\t\t\t<liana nLinks=\"%\" limitX=\"%\" limitZ=\"%\" />\n" \
					obj.links obj.limitX obj.limitZ to:fs
				VINE_Creep: (
					local parent = super.thisNode.parent
					if parent == undefined or classof parent != Plane then (
						messagebox ("Creep "+obj.name+" needs a Plane parent!") title:"Malformed level!" 
						format "\t\t\t<creep error=\"NO PLANE\" />\n" to:fs
					) else (
						format "\t\t\t<creep w=\"%\" h=\"%\"" parent.width parent.length to:fs
						writeTransform obj fs
						format " />\n" to:fs
					)
				)
				VINE_Pickup: format "\t\t\t<pickup typeIndex=\"%\" type=\"%\" strength=\"%\" />\n" \
					obj.type (obj.getType()) obj.strength to:fs
			)
			format "\t\t</object>\n" to:fs
			objectsExported += 1
		)
		
		format "\t</objects>\n" to:fs
	),
	
	fn exportSpecialCollisions fs quick:false tellCollisions:false = (
		local all = allSpecialCollisions()
		format "\t<collisions count=\"%\">\n" all.count to:fs
		for col in all do (
			local firstOfItsName = false -- king of the Andals and the Free Men...
			local special = getUserProp col "special"
			--remove quotation marks
			special = substring special 2 (special.count - 2)
			local meshName = "COLLISION_S_"+special
			format "\t\t<collision mesh=\"%\" " meshName to:fs
			local data_path = getDataPath ()
			if (findItem sessionCols colfilename) == 0 then (
				col.name = meshName
				if not quick then (
					local exporter = VINE_meshExporter()
					exporter.exportBasic col
				)
				firstOfItsName = true
				col.name = "COLLISION"
				append sessionCols colfilename
			)
			
			writeTransform col fs
			writeSpatialIndex col fs
			VINE_writeUserProps col fs:fs
			format " />\n" to:fs
			if firstOfItsName and tellCollisions then
				format "\t\t<!-- First special collision found: % -->\n" meshName to:fs
			collisionsExported += 1
		)
		format "\t</collisions>\n" to:fs
	),
	fn exportPieces fs quick:false tellCollisions:false = (
		local all = allPieces()
		format "\t<pieces count=\"%\">\n" all.count to:fs
		if all.count == 0 do format "WARNING: No pieces!\n"
		for p in all do (
			local kit = p.pieceKit
			local filename = p.pieceFilename
			local firstOfItsName = false -- king of the Andals and the Free Men...
			
			local taggers = for a in p.children where classof a == VINE_WildCard collect a
			
			-- find it

			format "\t\t<piece kit=\"%\" filename=\"%\"" kit filename x y z to:fs
			local meshName = "COLLISION_"+p.piecekit+"-"+p.piecefilename
			if (isTransformablePiece p or isSpecialPiece p or taggers.count > 0) \
			and not isGone p.collisionMesh then (
				local data_path = getDataPath ()
				local colfilename = data_path + "meshes\\"+meshName+".mesh"
				if (findItem sessionCols colfilename) == 0 then (
					p.collisionMesh.name = meshName
					if not quick then (
						local exporter = VINE_meshExporter()
						exporter.exportBasic p.collisionMesh
					)
					firstOfItsName = true
					p.collisionMesh.name = "COLLISION"
					append sessionCols colfilename
				)
				format " collision=\"%\"" meshName to:fs
			)
			
			for tag in taggers do (
				format " %=\"%\"" tag.tag1 tag.tag2 to:fs
			)
			
			writeTransform p fs
			writeSpatialIndex p fs
			local propsPath = VINE_meshesPath+"kits\\"+kit+"\\"+filename+".properties"
			if doesFileExist propsPath then (
				local file = openFile propsPath mode:"r"
				if file != undefined then (
					while not eof file do (
						local l = readLine file
						format " %" l to:fs
					) 
					close file
					gc()
				)
			)
			VINE_writeUserProps p fs:fs
			format " />\n" to:fs
			if firstOfItsName and tellCollisions then
				format "\t\t<!-- First collision found: % -->\n" meshName to:fs
			piecesExported += 1
		)
		format "\t</pieces>\n" to:fs
	),
	
	fn exportCollision name fs quick:false = (
		if not quick then (
			local collisionMesh = allCollisions transformables:false ()
			if meshop.getnumverts collisionMesh > 0 then (
				local obj = Editable_Mesh()
				obj.mesh = collisionMesh
				obj.name= name
				if meshop.getnumverts obj.mesh > 0 then (
					exporter = VINE_meshExporter()
					exporter.exportBasic obj
					delete obj
					format "\t<collision mesh=\"%\" />\n" name to:fs
				)
			)
			delete collisionMesh
		) else (
			format "\t<collision mesh=\"%\" />\n" name to:fs
		)
	),
	
	fn exportLevel name fs quick:false tellCollisions:false = (
		sessionCols = #()
		format "<!-- Autogenerated level file -->\n" to:fs
		format "<level name=\"%\">\n" name to:fs
		exportPieces fs quick:quick tellCollisions:tellCollisions
		exportEnemies fs quick:quick
		exportObjects fs quick:quick
		exportTriggers fs quick:quick
		exportSpatial fs quick:quick
		exportDestructibles fs quick:quick
		exportSmokeHazards fs quick:quick
		exportSpecialCollisions fs quick:quick
		exportWhiteBoxes fs quick:quick
		exportWildcards fs quick:quick
		exportWildcardboxes fs quick:quick
 		exportCollision (name+"_COLLISIONMESH") fs quick:quick
		format "</level>\n" to:fs
	),
	
	fn testExportScene name = (
		gc()
		local data_path = getDataPath ()
		
		local filename = data_path + "levels\\"
		filename += name + ".xml"
		
		clearListener()		
		-- Open file in binary mode, readOnly
		local fs = openFile filename mode:"r"
		if fs == undefined
			then (
				format "ERROR: can't open % for reading.\n" filename
				close fs
				)
			else format "Could open % for reading.\n" filename
		
		exportLevel name listener quick:true tellCollisions:true
		
		if piecesExported == 0 then (
			format "WARNING: no pieces exported.\n"
		)
		if objectsExported == 0 then (
			format "WARNING: no enemies exported.\n"
		)
		if enemiesExported == 0 then (
			format "WARNING: no objects exported.\n"
		)
		
		gc()
		close fs
		OK
	),
	
	fn exportScene name quick:false = (
		gc()
		local data_path = getDataPath ()
		
		local filename = data_path + "levels\\"
		filename += name + ".xml"
		
		-- Open file in binary mode, destroy existing version
		ensurePathExists(getFilenamePath(filename))
		local fs = openFile filename mode:"w"
		if fs == undefined then throw ( "ERROR: can't open "+filename+" for rewrite.")
		
		exportLevel name fs	quick:quick
		
		messagebox ("Exported level "+filename+" with: " \
			+(piecesExported as string)+" pieces, " \
			+(objectsExported as string)+" objects, " \
			+(enemiesExported as string)+" enemies, " \
			+(triggersExported as string)+" triggers, " \
			+(spatialExported as string)+" spatial areas, " \
			+(smokesExported as string)+" smokes, " \
			+(collisionsExported as string)+" special collisions, " \
			+(whiteboxesExported as string)+" whiteboxes, " \
			+(wildcardsExported as string)+" wildcard objects and " \
			+(wildcardBoxesExported as string)+" wildcard boxes." \
			) title:"Export complete." beep:false
		
		gc()
		close fs
		OK
	)
)

fn VINE_testExportLevel = (
	local exporter = VINE_exportLevel ()
	exporter.testExportScene (getFilenameFile maxfilename)
)

