function(symlink_datafiles target source dest)
	add_custom_command(
		TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${dest}
		DEPENDS ${source}
		COMMENT "symlink ${source} => ${dest}"
	)
endfunction()
