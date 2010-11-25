
if [ "$1" = "clean" ]; then
    make distclean
    exit 0
fi

make default
