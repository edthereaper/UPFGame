fileIn "tools.ms"

struct VINE_xsfBoneId (
	boneName,
	boneId
)

fn VINE_sort_influence a b = (
	case of (
		(a.weight < b.weight): 	return -1
		(a.weight > b.weight): 	return 1
		default: 				return  0
	)
)
	
struct VINE_influence (
	calBoneId,
	weight
)

struct VINE_meshExporter (
	
	version=5,

fn findIndexOfVertex vertices vertices_str v &vertexIds vId = (
	-- Convert my array of floats to a single string
	local sv = with printAllElements off v as string
	
	-- Look for the string in the array
	local index = findItem vertices_str sv
	
	if index > 0 then return index
	
	-- If it's not there, append it and return the index
	append vertices v
	append vertices_str sv
	append vertexIds vId
	return vertices.count
),

fn getFaceGroups obj = (
	
	local nfaces = getNumFaces obj
	local f = #()
	local face_id
	for face_id = 1 to nfaces do (
		local mat_id = getFaceMatID obj face_id
		-- If no face has been assigned to this material_id
		if f[ mat_id ] == undefined then (
			f[ mat_id ] = #()
		)
		append f[ mat_id ] face_id
	)
	return f
),

fn getDataPath = (
	local ret = getINISetting VINE_settingsFile "settings" "data_path"
	if ret == "" then throw("No data path set.")
	return ret
),

fn getToolsPath = (
	local ret = getINISetting VINE_settingsFile "settings" "tools_path"
	if ret == ""  then throw("No tools path set.")
	return ret
),

fn getThreshold = (
	local ret = getINISetting VINE_settingsFile "settings" "skin_threshold"
	if ret == "" then return 0.01
	else return ret
),

fn bytesPerVertex v = (
	local ans = 0
	for c in v do (
		case classof c of (
			float: ans+=4
			integer: ans+=4
			default:
				throw ("Unknown export value of class "+classof c)
		)
	)
	return ans
),

fn saveBinaryMesh obj vertices indices groups vertexType primitiveType = (

	local mesh_path = getDataPath ()
	
	local filename = mesh_path + "meshes\\"
	filename += obj.name + ".mesh"
	
	if vertices.count == 0 then throw ( "ERROR: no vertices.\n" )
	
	-- Open file in binary mode, destroy existing version
	ensurePathExists(getFilenamePath(filename))
	local fs = fopen filename "wb"
	if fs == undefined then throw ( "ERROR: can't open "+filename+" for rewrite.")
	
	local bytes_per_index = 2
	local bytes_per_vertex = bytesPerVertex vertices[1]
	
	local bbmin = max2mcv (obj.min - obj.pos)
	local bbmax = max2mcv (obj.max - obj.pos);
	minmax &bbmin &bbmax
	bbminv = #(bbmin.x, bbmin.y, bbmin.z)
	bbmaxv = #(bbmax.x, bbmax.y, bbmax.z)
	
	-- Save a header
	WriteLong fs TAG_mesh_begin
	WriteLong fs version
	WriteLong fs vertices.count
	WriteLong fs indices.count
	WriteLong fs primitiveType
	WriteLong fs bytes_per_vertex		-- # of bytes per vertex
	WriteLong fs bytes_per_index		-- # of bytes per index
	WriteLong fs vertexType
	for c in bbminv do (
		WriteFloat fs c
	)
	for c in bbmaxv do (
		WriteFloat fs c
	)
	WriteLong fs TAG_mesh_begin
	
	-- Vertices
	WriteLong fs TAG_mesh_vertices
	WriteLong fs (bytes_per_vertex*vertices.count)
	for v in vertices do (
		for c in v do (
			case classof c of (
				float: WriteFloat fs c
				--integer: 
				--	WriteLong fs c #unsigned
				default:
					throw ("Unknown export value of class "+classof c)
			
			)
		)
	)
	
	-- Indices
	WriteLong fs TAG_mesh_indices
	WriteLong fs (bytes_per_index*indices.count)
	for i in indices do (
		WriteShort fs (i-1) #unsigned		-- Pasar a base 0
	)
	
	-- Groups
	WriteLong fs TAG_mesh_groups
	WriteLong fs (groups.count*2*4)
	for g in groups do (
		WriteLong fs g[1]		-- First index (already in base0)
		WriteLong fs g[2]		-- Number of indices
	)
	
	-- Final chunk
	WriteLong fs TAG_mesh_end
	WriteLong fs 0
	
	format "Exported mesh % with % vertices and % indices in % groups.\n" \
		obj.name vertices.count indices.count groups.count
	
	fclose fs
),

fn getSecondTry obj submesh = ( 
	local m = obj.material
	
	if m == undefined then (
		return undefined
	) else (
		case classof m of (
			XRef_Material: return getfilenamepath m.srcFileName
			Multimaterial: (
				local sm = m[ submesh ]
				case classof sm of (
					XRef_Material: return getfilenamepath sm.srcFileName
					default:	return undefined
					)
				)
			default: return undefined
		)
	)
	return undefined
),

fn getMaterialOfObject obj submesh = ( 
	local m = obj.material
	
	if m == undefined then (
		format "WARNING: Submaterial %:% undefined.\n" obj.name (submesh as string)
		return undefined
	) else (
		case classof m of (
			Standardmaterial: return m
			XRef_Material: return m.GetSrcItem()
			Multimaterial: (
				local sm = m[ submesh ]
				case classof sm of (
					Standardmaterial: return sm
					XRef_Material: return sm.GetSrcItem()
					UndefinedClass: (
						format "WARNING: Submaterial %:% undefined.\n" obj.name (submesh as string)
						return undefined
					)
					Multimaterial: (
						format "WARNING: Submaterial %:% can't be a multimaterial itself.\n" obj.name (submesh as string)
						return undefined
					)
					default: (
						format "WARNING: Unsupported material type % on %.%\n" (classof sm) obj.name (submesh as string)
						return undefined
					)
				)
			)
			default: (
				format "WARNING: Unsupported material type % on %.%\n" (classof m) obj.name (submesh as string)
				return m
			)
		)
		
	)
	return undefined
),

fn exportUniqueRenderKeys obj matProps:#() = (
	gc()
	local mesh_path = getDataPath ()
	local filename = mesh_path + "prefabs\\mesh\\"
	filename += obj.name + ".xml"
	
	-- Open file in binary mode, destroy existing version
	ensurePathExists(getFilenamePath(filename))
	local fs = createfile filename
	if fs == undefined then throw ( "ERROR: can't open "+filename+" for rewrite.")
	
	format "<!--  Autogenerated prefab \"mesh\\" to:fs
	format "%\"  -->\n<Mesh name=\"%\">\n" obj.name obj.name to:fs
	
	local groups = getFaceGroups obj
	if classof obj.material == Standardmaterial then (
		format "\t<submesh id=\"%\" material=\"%\"/>\n"\
			0 obj.material.name \
			to:fs
	) else if classof obj.material == Multimaterial then (
		local sub_mesh = 0
		for m1 in obj.material do (
			if m1 != undefined then (
				local m = undefined
				local mat_id
				for mat_id = 1 to groups do (
					local m2 = getMaterialOfObject obj mat_id
					if m2 == m1 then (
						m = m2
						break
					)
				)
				local materialName = "default"
				if m != undefined then (
					materialName = m.name
				)
				format "\t<submesh id=\"%\" material=\"%\" maxMatId=\"%\" "\
					   sub_mesh materialName mat_id \
					   to:fs
				for prop in matProps do (
					if matchpattern l pattern:"sm*_*=\"*\"*" then (
						local tokenized = filterString prop "_"
						local count = tokenized.count
						local propMatId = substring (tokenized[1]) 3 -1
						if propMatId == (mat_id as string) then (
							for i = 2 to count do (
								if i < count
									then format "%_" tokenized[i] to:fs
									else format "% " tokenized[i] to:fs
							)
						)
					)
				)
				format "/>\n" to:fs
				sub_mesh = sub_mesh + 1
			)
		)
	) else (
		throw ("Unsupported material class " + (classof obj.material))
	)
		
	format "</Mesh>\n" to:fs	
),

fn getMatProps filePath:(maxfilepath + (getfilenamefile maxfilename)) = (
	local propsPath = filePath + ".properties"
	local matProps = #()
	if doesFileExist propsPath then (
		local propsFile = openFile propsPath mode:"r"
		if propsFile != undefined then (
			while not eof propsFile do (
				local l = readLine propsFile
				if  matchpattern l pattern:"mt*_*=\"*\"*" or \
					matchpattern l pattern:"sm*_*=\"*\"*" 
				then (
					append matProps l
				)
			)
			close propsFile
			gc()
		)
	)
	return matProps
),

fn exportRenderKeys obj  matProps:#() = (
	gc()
	local mesh_path = getDataPath ()
	local filename = mesh_path + "prefabs\\mesh\\"
	filename += obj.name + ".xml"
	
	-- Open file in binary mode, destroy existing version
	ensurePathExists(getFilenamePath(filename))
	local fs = createfile filename
	if fs == undefined then throw ( "ERROR: can't open "+filename+" for rewrite.")
		
	local groups = getFaceGroups obj
	local sub_mesh = 0
	local mat_id = 1
	format "<!--  Autogenerated prefab \"mesh\\" to:fs
	format "%\"  -->\n<Mesh name=\"%\">\n" obj.name obj.name to:fs
	
	for g in groups do (
		if g != undefined then (
			local m = getMaterialOfObject obj mat_id
			local materialName = "default"
			if m != undefined then (
				materialName = m.name
			)
			format "\t<submesh id=\"%\" material=\"%\" maxMatId=\"%\" " sub_mesh materialName mat_id to:fs
			for prop in matProps do (
				if matchpattern prop pattern:"sm*_*=\"*\"*" then (
					local tokenized = filterString prop "_"
					local count = tokenized.count
					local propMatId = substring (tokenized[1]) 3 -1
					if propMatId == (mat_id as string) then (
						for i = 2 to count do (
							if i < count
								then format "%_" tokenized[i] to:fs
								else format "% " tokenized[i] to:fs
						)
					)
				)
			)
			format "/>\n" to:fs
			sub_mesh = sub_mesh + 1
		)
		mat_id = mat_id + 1
	)
		
	format "</Mesh>\n" to:fs	
),

fn importTexture infile secondTry:undefined = (
	local out_path   = getDataPath()
	local tools_path = getToolsPath()
	local filename = infile
	local base_name = getFilenameFile infile
	if not doesFileExist(filename) then (
		filename =  maxfilepath + base_name + (getFilenameType infile) 
		format "Trying to find texture % in: %\n" base_name filename
	)
	if not doesFileExist(filename) and secondTry != undefined then (
		filename =  secondTry + base_name + (getFilenameType infile) 
		format "Trying to find texture % in: %\n" base_name filename
	)
	if not doesFileExist(filename) then (
		format "WARNING: texture % seems to be missing.\n" base_name
	)
	local full_target = out_path + "textures/" + base_name + ".dds"
	
	-- Convert to dds using texconv.exe
	ensurePathExists(getFilenamePath(full_target))
	local cmd =
		tools_path + "texconv.exe -nologo " +
		"-f DXT5 \""+ filename + "\" " +
		"-o " + out_path + "textures\\"
	format "exec> % \n" cmd
	local log_file = tools_path + "export.log"
	if doesfileExist log_file then deleteFile log_file
	local rc = DOSCommand (cmd + " > " + log_file)
	if rc != 0 then format "WARNING: check %%\n" tools_path "export.log for errors"
	cat log_file
	format "\n" cmd
	return base_name
),

fn importTextureMap map mapName fullName mfs secondTry:undefined = (
	if map == undefined then format "WARNING: no %.\n" fullName
	else (
		local filePath = importTexture map.filename secondTry:secondTry
		format "%=\"%\" " mapName filePath to:mfs
	)
),

fn getMatIdsOfMat obj mat = (
	local groups = getFaceGroups obj	
	local mat_id = 1
	local ans = #()
	for g in groups do (
		local m = getMaterialOfObject obj mat_id
		if m == mat then append ans mat_id
		mat_id+=1
	)
	return ans
),

fn importMaterial obj m matId tech secondTry:undefined matProps:#() = (
	if export_materials == false then return 0
	if classof m != Standardmaterial then (
		format "WARNING: unsupported material type %.\n" ((classof m) as string)
		return 0
	)
	
	local out_path = getDataPath()
	local out_filename = out_path + "\\materials\\" + m.name + ".xml"
	ensurePathExists(getFilenamePath(out_filename))
	local mfs = createfile out_filename
	if mfs == undefined then throw ("Can't create material file " + out_filename + ".")
	format "<std_material name=\"%\" " m.name to:mfs
	
	-- diffuse
	importTextureMap m.diffusemap "diffuse" "diffuse texture" mfs secondTry:secondTry
	
	-- normal map
	local bump = m.bumpmap
	if classof bump == Normal_Bump then (
		importTextureMap bump.normal "normal" "normal map" mfs secondTry:secondTry
	) else (
		importTextureMap bump "normal" "normal map" mfs secondTry:secondTry
	)
	
	-- selfIllumination
	importTextureMap m.selfIllumMap "selfIllumination" "self-illumination texture" mfs secondTry:secondTry
	
	if tech != undefined then (
		format " tech=\"%\" " tech to:mfs
	)
	
	local sharedIds = getMatIdsOfMat obj m
	for prop in matProps do (
		if matchpattern prop pattern:"mt*_*=\"*\"*" then (
			local tokenized = filterString prop "_"
			local count = tokenized.count
			local propMatId = substring (tokenized[1]) 3 -1
			for id in sharedIds do (
				if propMatId == (id as string) then (
					for i = 2 to count do (
						if i < count
							then format "%_" tokenized[i] to:mfs
							else format "% " tokenized[i] to:mfs
					)
					break
				)
			)
		)
	)
	
	format " />\n" to:mfs
	close mfs
	return unique
),

fn importMaterials obj matProps:#() tech:undefined filepath:(maxfilepath + (getfilenamefile maxfilename)) = (
	local groups = getFaceGroups obj
	local mat_id = 1
	
	for g in groups do (
		if g != undefined then (
			local m = getMaterialOfObject obj mat_id
			local materialName = "default"
			if m != undefined then (
				local secondTry
				if filepath == undefined
					then secondTry = getSecondTry obj mat_id
					else secondTry = filepath
				importMaterial obj m mat_id tech secondTry:secondTry matProps:matProps
			)
		)
		mat_id = mat_id + 1
	)
),

fn computeTangentSpace &vtxs &idxs = (
		local vertexCount = vtxs.count
	local tan1 = #()
	local tan2 = #()

	-- default arrays to zero
	for i = 1 to vertexCount do (
		tan1[i] = [0,0,0]
		tan2[i] = [0,0,0]
	)

	local a = 1
	while( a < idxs.count ) do (
		-- Get the indices of each triangle in base 1
		local i1 = idxs[a]
		local i2 = idxs[a+1]
		local i3 = idxs[a+2]
		
		-- vtx-i is the original array of floats #(pos, uv, N ...)
		local vtx1 = vtxs[ i1 ]	
		local vtx2 = vtxs[ i2 ]
		local vtx3 = vtxs[ i3 ]
		
		-- The positions are the 3 first floats in each array
		local v1 = [vtx1[1], vtx1[2], vtx1[3] ]
		local v2 = [vtx2[1], vtx2[2], vtx2[3] ]
		local v3 = [vtx3[1], vtx3[2], vtx3[3] ]

		-- Then the tex coords
		local w1 = [vtx1[4], vtx1[5]]
		local w2 = [vtx2[4], vtx2[5]]
		local w3 = [vtx3[4], vtx3[5]]
		        
		local x1 = v2.x - v1.x
        local x2 = v3.x - v1.x
        local y1 = v2.y - v1.y
        local y2 = v3.y - v1.y
        local z1 = v2.z - v1.z
        local z2 = v3.z - v1.z
        
        local s1 = w2.x - w1.x
        local s2 = w3.x - w1.x
        local t1 = w2.y - w1.y
        local t2 = w3.y - w1.y
		
		local r = 1.0 / (s1 * t2 - s2 * t1)

		local sdir = [(t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r]
        local tdir = [(s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r]

		tan1[i1] = tan1[i1] + sdir
		tan1[i2] = tan1[i2] + sdir
		tan1[i3] = tan1[i3] + sdir
		                     
		tan2[i1] = tan2[i1] + tdir
		tan2[i2] = tan2[i2] + tdir
		tan2[i3] = tan2[i3] + tdir

		-- Next triangle
		a = a + 3
	)
    
	
	for a = 1 to vtxs.count do (
		local n = [ vtxs[a][6], vtxs[a][7], vtxs[a][8] ]
		local t = tan1[a]
		
        -- Gram-Schmidt orthogonalize
		local dot_nt = dot n t
        local tangent = normalize( t - n * dot_nt)
		
        -- Calculate handedness
		local cross_nt = cross n t
		local dot_cross_tan2 = dot cross_nt tan2[a]
        local tangent_w = 1.0
		if dot_cross_tan2 < 0 then (
			tangent_w = -1.0
		)
		
		-- Append the tangent + tanget_w to the information of each vertex
		join vtxs[a] #( tangent.x, tangent.y, tangent.z, tangent_w )
	)
),

fn getXSFBones filename = (/*
	xmldoc = dotnetobject "System.XML.XMLDocument"
	xmldoc.load filename

	boneIds = #()
	
	itemIter = (xmldoc.selectnodes "//SKELETON/BONE").GetEnumerator()
	while itemIter.MoveNext() do (
		attrIter = itemIter.Current.Attributes.GetEnumerator()
		local entry = VINE_xsfBoneId()
		while attrIter.MoveNext() do (
			local att = attrIter.current.Name
			local val = attrIter.current.value
			case att of (
				"NAME": entry.boneName = val
				"ID": entry.boneId = val as integer
			)
		)
		append boneIds entry
	)
	return boneIds*/
),


fn findCalBoneId boneName calBones = (/*
	for calBone in calBones do(
		if (calBone.boneName == boneName)
		then (
			return calBone.boneId
		)
	)
	throw ("ERROR! Bone "+boneName+" not found.")*/
),

fn getBoneName skinning boneId = (/*
	-- compatibility
	local mversion = (maxVersion())[1]
	if mversion >= 17000 then (
		-- 3ds max 2015
		return skinOps.getBoneName skinning boneId 1
	) else (
		throw "Unsupported version"
	)*/
),

fn exportSkinning objExport xsf vertices vertexIds = (	/*
	-- Confirm obj is valid
	if objExport == undefined then throw "ERROR: obj is undefined" 
		
	local baseObject = objExport.baseObject
	local skinning = objExport.modifiers[#Skin]
	if (skinning == undefined) then throw (objExport + " has no skinning!")
	
	local maxBones = 4
	local threshold = 0.01
	local calBones = getXSFBones xsf
	
	-- (Tell artists to select the modifier panel before exporting!)
	if modPanel.getCurrentObject() == undefined then (
		throw "The \"Modify\" panel must be selected before exporting the skinning!"
	)
	-- Skin need to be selected and in the modifier panel
	modPanel.setCurrentObject skinning
	
	for i = 1 to vertices.count do (
		local vId = vertexIds[i]
		local v = vertices[i]
		local influences = #()
		nInfluences = skinOps.GetVertexWeightCount skinning vId
		for nInfluence=1 to nInfluences do (
			local boneId = skinOps.GetVertexWeightBoneID skinning vId nInfluence
			local weight = skinOps.GetVertexWeight skinning vId nInfluence
			local boneName = getBoneName skinning boneId
			local influence =  VINE_influence()
			influence.calBoneId = findCalBoneId boneName calBones
			influence.weight = weight
			append influences influence
		)
		local boneIds = 0 
		local boneWeights = 0
		qsort influences VINE_sort_influence
		
		nBones =  maxBones
		if influences.count < maxBones then nBones = influences.count
		
		local wSum = 0
		for nInfluence=1 to nBones do (
			wSum += influences[nInfluence].weight
		)
		for nInfluence=1 to nBones do (
			influences[nInfluence].weight /= wSum
		)
			
		local encodedWeights = #()
		local encodedBones = #()
		local wSumi = 0
		for nInfluence=1 to maxBones do (
			if nInfluence <= influences.count then (
				local wf = influences[nInfluence].weight
				local wi
				if nInfluence == influences.count
				then wi = 255 - wSumi
				else wi = bit.and (integer(wf*256)) 0xFF
				wSumi += wi
				append encodedWeights wi
				append encodedBones (bit.and influences[nInfluence].calBoneId 0xFF)
			) else (
				append encodedWeights 0
				append encodedBones 0xFF
			)
		)
		
		boneIds = bit.or boneIds (bit.shift encodedBones[1] 24 )
		boneIds = bit.or boneIds (bit.shift encodedBones[2] 16)
		boneIds = bit.or boneIds (bit.shift encodedBones[3] 8)
		boneIds = bit.or boneIds (bit.shift encodedBones[4] 0)
		boneWeights = bit.or boneWeights (bit.shift encodedWeights[1] 24)
		boneWeights = bit.or boneWeights (bit.shift encodedWeights[2] 16)
		boneWeights = bit.or boneWeights (bit.shift encodedWeights[3] 8)
		boneWeights = bit.or boneWeights (bit.shift encodedWeights[4] 0)
		join v #(boneIds, boneWeights)
	)
	
	modPanel.setCurrentObject baseObject*/
),

fn getBasicMesh objExport &rvertices &rindices &rgroups &rvertexIds &rvertexType &rprimitiveType = (
	
	-- Confirm obj is valid
	if objExport == undefined then throw "ERROR: obj is undefined" 
	local obj = objExport
	if not (canConvertTo obj Editable_Mesh) then (
		throw "ERROR: can't convert object to editable mesh"
	)
	if classof objExport  != Editable_mesh then (
		obj = copy objExport
		obj.name = objExport.name
		convertToMesh obj
	)
	
	local world2mcv_m = max2mcv (inverse obj.transform)
	
	-- Vertices as string
	local vertices_str = #() 
	local primitiveType = PRIMITIVE_triangle_list
	local vertices = #()
	local vertexIds = #()
	local indices = #()
	local groups = #()
	
	local nfaces = getNumFaces obj
		
	local channelA = 1
	
	--For each group of faces (material)
	local faceGroups = getFaceGroups obj
	local faceGroup
	for faceGroup in faceGroups do (
		if (faceGroup == undefined) then continue
		
		-- Current position in indices array
		local begin = indices.count
		
		-- For each face
		local face
		for face in faceGroup do (
			-- Position
			local pos = getFace obj face
			local normals = meshop.getFaceRNormals obj face
			
			-- For the 3 vtxs of each face (counter-clockwise)
			local vId
			for vId in #(1,3,2) do (w
				local wPos = meshop.getVert obj pos[vId]
				local mPos = wPos * world2mcv_m 
				
				-- Collect all data associated to this vtx
				local v = #( mPos.x, mPos.y, mPos.z)
				
				-- Index the vtx or get the index if the vtx has already been defined
				local index = findIndexOfVertex vertices vertices_str v vertexIds pos[vId]
				append indices index
			)
		)
		
		-- First index & number of indices
		local g = #( begin, faceGroup.count * 3 )
		append groups g
	)
	
	-- Delete the temp mesh
	if obj != objExport then delete obj
	
	local vertexType = VERTEX_pos
		
	-- Returned values
	rvertices = vertices
	rindices = indices
	rgroups = groups
	rvertexIds = vertexIds
	rvertexType = vertexType
	rprimitiveType = primitiveType
),

fn exportMesh objExport &rvertices &rindices &rgroups &rvertexIds &rvertexType &rprimitiveType = (
	
	-- Confirm obj is valid
	if objExport == undefined then throw "ERROR: obj is undefined" 
	local obj = objExport
	if not (canConvertTo obj Editable_Mesh) then (
		throw "ERROR: can't convert object to editable mesh"
	)
	if classof objExport  != Editable_mesh then (
		obj = copy objExport
		obj.name = objExport.name
		convertToMesh obj
	)
	
	local world2mcv_m = max2mcv (inverse obj.transform)
	
	-- Vertices as string
	local vertices_str = #() 
	local primitiveType = PRIMITIVE_triangle_list
	local vertices = #()
	local vertexIds = #()
	local indices = #()
	local groups = #()
	
	local nfaces = getNumFaces obj
		
	local channelA = 1
	
	--For each group of faces (material)
	local faceGroups = getFaceGroups obj
	local faceGroup
	for faceGroup in faceGroups do (
		if (faceGroup == undefined) then continue
		
		-- Current position in indices array
		local begin = indices.count
		
		-- For each face
		local face
		for face in faceGroup do (
			-- Position
			local pos = getFace obj face
			local mapA = meshop.getMapFace obj channelA face
			local normals = meshop.getFaceRNormals obj face
			
			-- For the 3 vtxs of each face (counter-clockwise)
			local vId
			for vId in #(1,3,2) do (
				local wPos = meshop.getVert obj pos[vId]
				local uvA = meshop.getMapVert obj channelA mapA[vId]
				local wn = normals[vId]
				local mPos = wPos * world2mcv_m 
				local n = wn * world2mcv_m.rotationpart 
				
				-- Collect all data associated to this vtx
				local v = #( mPos.x, mPos.y, mPos.z, uvA.x, 1-uvA.y, n.x, n.y, n.z )
				
				-- Index the vtx or get the index if the vtx has already been defined
				local index = findIndexOfVertex vertices vertices_str v vertexIds pos[vId]
				append indices index
			)
		)
		
		-- First index & number of indices
		local g = #( begin, faceGroup.count * 3 )
		append groups g
	)
	
	-- Delete the temp mesh
	if obj != objExport then delete obj
		
	computeTangentSpace vertices indices
	local vertexType = VERTEX_pos_uv_normal_tangent
		
	-- Returned values
	rvertices = vertices
	rindices = indices
	rgroups = groups
	rvertexIds = vertexIds
	rvertexType = vertexType
	rprimitiveType = primitiveType
),

fn exportBasic objExport = (
	-- Confirm obj is valid
	if objExport == undefined then throw "ERROR: no object selected" 
	local obj = objExport
	if not (canConvertTo obj Editable_Mesh) then (
		throw ("ERROR: can't convert "+objExport+" to editable mesh")
	)
	if classof objExport  != Editable_mesh then (
		obj = copy objExport
		obj.name = objExport.name
		convertToMesh obj
	)
	
	local vertices = #()
	local indices = #()
	local groups = #()
	local vertexIds = #()
	local vertexType = 0
	local primitiveType = 0
	
	getBasicMesh obj &vertices &indices &groups &vertexIds &vertexType &primitiveType
	-- Save in binary form to file
	saveBinaryMesh obj vertices indices groups vertexType primitiveType
	
	-- Delete the temp mesh<
	if obj != objExport then delete obj
	OK
),

fn exportStatic objExport = (
	-- Confirm obj is valid
	if objExport == undefined then throw "ERROR: no object selected" 
	local obj = objExport
	if not (canConvertTo obj Editable_Mesh) then (
		throw ("ERROR: can't convert "+objExport+" to editable mesh")
	)
	if classof objExport  != Editable_mesh then (
		obj = copy objExport
		obj.name = objExport.name
		convertToMesh obj
	)
	
	local vertices = #()
	local indices = #()
	local groups = #()
	local vertexIds = #()
	local vertexType = 0
	local primitiveType = 0
	
	exportMesh obj &vertices &indices &groups &vertexIds &vertexType &primitiveType
	-- Save in binary form to file
	saveBinaryMesh obj vertices indices groups vertexType primitiveType
	
	-- Delete the temp mesh<
	if obj != objExport then delete obj
	OK
),

fn exportMaterials objExport filepath:(maxfilepath + (getfilenamefile maxfilename)) = (
	-- Confirm obj is valid
	if objExport == undefined then throw "ERROR: no object selected" 
		
	local obj = objExport
	if not (canConvertTo obj Editable_Mesh) then (
		throw ("ERROR: can't convert "+objExport+" to editable mesh")
	)
	if classof objExport  != Editable_mesh then (
		obj = copy objExport
		obj.name = objExport.name
		convertToMesh obj
	)
	
	local skinned = objExport.modifiers[#Skin] != undefined
	local matProps = getMatProps filePath:filePath ()	
	if skinned then (
		exportUniqueRenderKeys obj skinned:skinned matProps:matProps
		importMaterials obj tech:"skin" filepath:filepath matProps:matProps
	) else (
		exportRenderKeys obj matProps:matProps
		importMaterials obj filepath:filepath matProps:matProps
	)
	
	-- Delete the temp mesh
	if obj != objExport then delete obj
	OK
)

) --struct VINE_staticMeshExporter end

fn LOAD_MACRO_VINE_staticMeshExporter = (
	macroScript MACRO_VINE_staticMeshExporter
		category:"VINEDETTA"
		toolTip: "Export static mesh"
		icon:#("exportMesh", 1)
	(
		exporter = VINE_meshExporter()
		exporter.exportStatic $
	)
	OK
)
LOAD_MACRO_VINE_staticMeshExporter()

fn LOAD_MACRO_VINE_materialExporter = (
	macroScript MACRO_VINE_materialExporter
		category:"VINEDETTA"
		toolTip: "Export materials"
		icon:#("exportMat", 1)
	(
		exporter = VINE_meshExporter()
		exporter.exportMaterials $
	)
	OK
)
LOAD_MACRO_VINE_materialExporter()

