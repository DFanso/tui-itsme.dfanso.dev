function(itsme_set_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /utf-8 /permissive-
      $<$<BOOL:${ITSME_WERROR}>:/WX>)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic
      $<$<BOOL:${ITSME_WERROR}>:-Werror>)
  endif()
endfunction()
