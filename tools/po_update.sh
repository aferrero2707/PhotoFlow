#! /bin/bash

rm -f /tmp/sources.list
dirs="base operations gui"
for dir in $dirs; do
	find src/$dir -iname "*.h" >> /tmp/sources.list
        find src/$dir -iname "*.hh" >> /tmp/sources.list
        find src/$dir -iname "*.c" >> /tmp/sources.list
        find src/$dir -iname "*.cc" >> /tmp/sources.list
done
xgettext --from-code=UTF-8 --files-from=/tmp/sources.list --default-domain=photoflow -k_ --join-existing -o po/photoflow.pot

langs="fr"
for lang in $langs; do
    msgmerge --output-file=po/${lang}.po po/${lang}.po po/photoflow.pot
done
#rm -f /tmp/sources.list

