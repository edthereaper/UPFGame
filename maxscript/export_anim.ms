filein "tools.ms"

struct VINE_AnimExporter(

	fn getDataPath = (
		local ret = getINISetting VINE_settingsFile "settings" "data_path"
		if ret == "" then throw("No data path set.")
		return ret
	),

	fn exportAnim object1 = (

			--valid object1
		if object1 == undefined then throw "I need a valid object!"
		
			--path of the camera file
		/*local data_path = "C:\\escenario\\data\\"
		local out_path = data_path + "camera\\"*/
		local data_path = getDataPath()
		local out_path = data_path + "animation\\"
		
		ensurePathExists(getFilenamePath(out_path))
		
		local filename = out_path + object1.name + ".anim"
		local count = 1
		
		local fs = fopen filename "w+"
		if fs == undefined then throw ( "Can't create object animfile output file " + filename )

		-- identify
		local object_magic = 0x00aaaaa1
		
		-- storage keyframes
		local nkeys = animationRange.end - animationRange.start + 1
		local animation_duration = ( nkeys as float ) / 4800		-- Convert frames to ticks and ticks to second
		-- Save header
		WriteLong fs object_magic
		WriteLong fs nkeys 						-- per object
		WriteFloat fs animation_duration		-- In seconds
		WriteLong fs 0 

		-- Write for each key, where is each objs
		local w = animationRange.start
		while w <= animationRange.end do (
			
			--format "Exporting key %\n"w
			-- For each obj
		
				local posObj = at time w object1.position	
				local posObjmcv = max2mcv posObj
				
				WriteFloat fs posObjmcv.x
				WriteFloat fs posObjmcv.y
				WriteFloat fs posObjmcv.z
				WriteFloat fs 0

				local rotObj = at time w object1.rotation	
				local rotObjmcv = max2mcvQuat rotObj

				WriteFloat fs rotObjmcv.x
				WriteFloat fs rotObjmcv.y
				WriteFloat fs rotObjmcv.z
				WriteFloat fs rotObjmcv.w

				format "At Time % pos:% rot:%\n" w  posObjmcv rotObjmcv
			
			w = w + 1
		)
		
		format "Exporting % camera and target with % keys\n" object1.name nkeys
		-- Cerrar fichero
		fclose fs
		OK
	),
	
	fn exportXML object1 = (
		
		--path of the camera file
		local data_path = getDataPath()
		local out_path = data_path + "animation\\"
		
		ensurePathExists(getFilenamePath(out_path))
		
		local file_name = out_path + getFilenameFile object1.name + ".xml"
		
		-- Open file in binary mode, destroy existing version
		local fs_exporter = openFile file_name mode:"w+"
		if fs_exporter == undefined then throw ( "ERROR: can't open "+file_name+" for rewrite.")
		
		format "<anim>\n" to:fs_exporter
		local maxFrame = getUserProp object1 "max"
		format "\t<MaxAnim name=\"%\" max=\"%\" type=\"\" isOn=\"0\"/>\n" object1.name maxFrame to:fs_exporter
		format "</anim>" to:fs_exporter
		close fs_exporter
			
		messageBox "XML generated successfull"
	)
)

fn TEST_VINE_AnimExport object1 = (

		local exporter = VINE_AnimExporter()
		exporter.exportAnim(object1)
		exporter.exportXML(object1)
		OK
)

--TEST_VINE_AnimExport($)