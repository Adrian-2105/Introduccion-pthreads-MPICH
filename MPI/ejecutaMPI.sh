#!/bin/bash

if (! test $# -eq 1)
then
    echo "Uso: $0 [EXECUTABLE]" >&2
    exit 1
fi

if (! test -f $1 -o -r $1)
then
    echo "El fichero '$1' no existe o no se puede leer" >&2
    exit 2
fi

if (! test -f machinefile -o -r machinefile)
then
    echo "El fichero machinefile no existe o no se puede leer" >&2
    exit 2
fi

mpirun -f machinefile ./$1
