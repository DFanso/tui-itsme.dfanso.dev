function(itsme_set_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /utf-8 /permissive-
      $<$<BOOL:${ITSME_WERROR}>:/WX>)
    # std::getenv / std::localtime are used deliberately; silence MSVC's C4996 deprecation.
    target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic
      $<$<BOOL:${ITSME_WERROR}>:-Werror>)
  endif()
endfunction()
