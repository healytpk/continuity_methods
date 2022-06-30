#!/bin/bash

# Usage: thomas_egypt_Keep_If_In_Scopes.bash std __gnu_cxx zXdjMainsdi
FUNCTION_NAME_WITH_SCOPE_PREFIX=$( c++filt -p -n ${@: -1} 2>&1 )

SCOPE_NAMES=${@:1:$#-1}

for arg in $SCOPE_NAMES; do
	
	#printf "Checking if prefixed with $arg:: \n"

	if  [[ `thomas_egypt_get_scope "$FUNCTION_NAME_WITH_SCOPE_PREFIX"` =~ ^$arg ]]; then

		echo ${@: -1}
		exit 0
	fi

done

exit 0
