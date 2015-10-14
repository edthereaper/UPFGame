filein "tools.ms"
filein "export_mesh.ms"

fn VINE_BatchExport dir = (
	with undo off (
		with redraw off (
			local sel = $
			if sel != undefined then sel = for obj in $ collect obj
			local deep = deepDir dir "*.max"
			local failedFiles = #()
			local failedObjs = #()
			local exporter = VINE_meshExporter()
			for file in deep do (
				try (
					format "%\n" file
					mergemaxfile file #select #mergeDups #useMergedMtlDups
					local merged = $
					for obj in merged where canConvertTo obj Editable_Mesh do (
						try (
							convertTo obj Editable_Mesh
							format "Exporting %\n" obj.name
							exporter.exportStatic obj
							exporter.exportMaterials obj filepath:(getFilenamePath file)
							format "\n"
						) catch (
							local objName
							if not isGone obj
								then (
									objName = "["+(getfilenameFile file)+".max]."+obj.name
									delete obj
								)
								else objName = "["+(getfilenameFile file)+".max].???"
							append failedObjs objName
							format "Unexpected error while exporting %, aborted.\n\n" objName
						)
					)
					for obj in merged do if not isGone obj then delete obj
				) catch (
					append failedFiles file
					format "Unexpected error while exporting file %.\n\n" file
				)
			)
			format "\n\n-------------------------------\n\t BATCH EXPORT COMPLETE\n-------------------------------\n\n"
			if failedFiles.count > 0 then (
				format "The following files caused exceptions:\n"
				for file in failedFiles do (
					format "\t%\n" file
				)
			)
			if failedObjs.count > 0 then (
				format "\nThe following objects caused exceptions:\n"
				for obj in failedObjs do (
					format "\t%\n" obj
				)
			)
			if sel != undefined then select sel
		)
	)
	OK
)

fn LOAD_MACRO_VINE_BatchExport = (
	macroScript MACRO_VINE_BatchExport
		category:"VINEDETTA"
		toolTip: "Batch export meshes and materials"
		icon:#("exportBatch", 1)
	(
		local dir = getSavePath \
				caption:"Select the folder to be scanned for .max files" \
				initialDir:maxfilepath historyCategory:"VINE_BatchExport"
			if dir != undefined
				then VINE_BatchExport dir
				else throw "No valid path selected!"
	)
	OK
)
LOAD_MACRO_VINE_BatchExport()
