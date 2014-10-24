#! /bin/bash

conf="$1"

nl=$(cat "$conf" | wc -l)

op_name=""
op_name2=""
command=""
title=""
rm -f par_def.cpp
rm -f widgets_def.cpp
echo -n "  iterations(\"iterations\",this,1)" > par_imp.cpp
echo -n "  iterations_slider( this, \"iterations\", \"Iterations\", 1, 1, 10, 1, 1, 1)" > widgets_imp.cpp
rm -f par_imp2.cpp
rm -f widgets_imp2.cpp
rm -f command.cpp
npar=0
for i in $(seq 1 $nl); do

    line=$(cat "$conf" | sed -n ${i}p)
    name=$(echo "$line" | cut -d"=" -f 1)
    value=$(echo "$line" | cut -d"=" -f 2)

    if [ x"$name" = "xop_name" ]; then
        op_name="$value"
        op_name_uc="${op_name^^[a-z]}"
        echo "$op_name ${op_name_uc}"
    fi

    if [ x"$name" = "xop_name2" ]; then
        op_name2="$value"
        echo "${op_name2}"
    fi

    if [ x"$name" = "xcommand" ]; then
        command="$value "
        echo "${command}"
        echo "  std::string command = \"-$command \";" >> command.cpp
    fi

    if [ x"$name" = "xtitle" ]; then
        title="$value "
        echo "${title}"
    fi

    if [ x"$name" = "xparameter" ]; then

        echo "," >> par_imp.cpp
        echo "," >> widgets_imp.cpp

        type=$(echo "$value" | tr -s " " | cut -d" " -f 1)
        pname=$(echo "$value" | tr -s " " | cut -d" " -f 2 | cut -d"[" -f 1)
        if [ x"$type" = "xenum" ]; then

            echo "    PropertyBase prop_${pname};" >> par_def.cpp
            echo "    Selector prop_${pname}_selector;" >> widgets_def.cpp
            pdef=$(echo "$value" | tr -s " " | cut -d" " -f 3)
            enumvalues=$(echo "$value" | tr -s " " | cut -d" " -f 2 | cut -d"[" -f 2 | cut -d"]" -f 1)
            echo "$type $pname [${enumvalues}] $pdef"
            nval=$(echo "$enumvalues" | tr "," "\n" | wc -l)
            jdef=0
            defval=""
            defval2=""
            for j in $(seq 1 $nval); do
                enumval=$(echo "$enumvalues" | cut -d"," -f $j)
                enumval2=$(echo "$enumval" | tr "-" "_")
                if [ "$enumval" = "$pdef" ]; then 
                    jdef=$j; 
                    defval="$enumval"
                    defval2="$enumval2"
                else
                    echo "  prop_${pname}.add_enum_value( $j, \"$enumval\", \"$enumval\" );" >> par_imp2.cpp
                fi
            done
            echo -n "  prop_${pname}(\"$pname\", this, $jdef, \"$defval\", \"$defval\")" >> par_imp.cpp
            echo -n "  prop_${pname}_selector( this, \"$pname\", \"$pname\", ${jdef})" >> widgets_imp.cpp
            echo "  controlsBox.pack_start( prop_${pname}_selector );" >> widgets_imp2.cpp

        else

            echo "    Property<${type}> prop_${pname};" >> par_def.cpp
            echo "    Slider prop_${pname}_slider;" >> widgets_def.cpp
            pmin=$(echo "$value" | tr -s " " | cut -d" " -f 3)
            pmax=$(echo "$value" | tr -s " " | cut -d" " -f 4)
            pdef=$(echo "$value" | tr -s " " | cut -d" " -f 5)
            echo "$type $pname $pmin $pmax $pdef"
            echo -n "  prop_${pname}(\"$pname\",this,${pdef})" >> par_imp.cpp
            if [ x"$type" = "xint" ]; then
                echo -n "  prop_${pname}_slider( this, \"$pname\", \"$pname\", ${pdef}, ${pmin}, ${pmax}, 1, 5, 1)" >> widgets_imp.cpp
            else
                pdelta1=$(echo "scale=1; ($pmax-$pmin)/10" | bc -l)
                pdelta2=$(echo "scale=2; ($pmax-$pmin)/100" | bc -l)
                echo -n "  prop_${pname}_slider( this, \"$pname\", \"$pname\", ${pdef}, ${pmin}, ${pmax}, ${pdelta2}, ${pdelta1}, 1)" >> widgets_imp.cpp
            fi
            echo "  controlsBox.pack_start( prop_${pname}_slider );" >> widgets_imp2.cpp

        fi

        if [ $npar -gt 0 ]; then
            echo "  command = command + std::string(\",\") + prop_${pname}.get_str();" >> command.cpp
        else
            echo "  command = command + prop_${pname}.get_str();" >> command.cpp
        fi

        npar=$((npar+1))

    fi

done

echo "" >> par_imp.cpp
echo "" >> widgets_imp.cpp

cp op.hh tmp.hh
cat tmp.hh | sed "s/%op_name_uc%/${op_name_uc}/" > tmp2.hh
cp tmp2.hh tmp.hh
cat tmp.hh | sed "s/%op_name%/${op_name}/" > tmp2.hh
cp tmp2.hh tmp.hh
cat tmp.hh | sed "s/%op_name2%/${op_name2}/" > tmp2.hh

cp tmp2.hh tmp.hh
rm -f tmp2.hh
par_def=$(cat par_def.cpp)
echo "$par_def"
nl=$(cat tmp.hh | wc -l)
for i in $(seq 1 $nl); do
    line=$(cat tmp.hh | sed -n ${i}p)
    test=$(echo "$line" | grep '%par_def%')
    if [ x"$test" = "x" ]; then
        echo "$line" >> tmp2.hh
    else
        cat par_def.cpp >> tmp2.hh
    fi
done
cp tmp2.hh "../../../operations/gmic/${op_name}.hh"


cp op.cc tmp.cc
cat tmp.cc | sed "s/%op_name_uc%/${op_name_uc}/" > tmp2.cc
cp tmp2.cc tmp.cc
cat tmp.cc | sed "s/%op_name%/${op_name}/" > tmp2.cc
cp tmp2.cc tmp.cc
cat tmp.cc | sed "s/%op_name2%/${op_name2}/g" > tmp2.cc

cp tmp2.cc tmp.cc
rm -f tmp2.cc
par_imp=$(cat par_imp.cpp)
echo "$par_imp"
nl=$(cat tmp.cc | wc -l)
for i in $(seq 1 $nl); do
    line=$(cat tmp.cc | sed -n ${i}p)
    test=$(echo "$line" | grep '%par_imp%')
    if [ x"$test" != "x" ]; then
        cat par_imp.cpp >> tmp2.cc
        continue;
    fi
    test=$(echo "$line" | grep '%par_imp2%')
    if [ x"$test" != "x" ]; then
        cat par_imp2.cpp >> tmp2.cc
        continue;
    fi
    test=$(echo "$line" | grep '%command%')
    if [ x"$test" != "x" ]; then
        cat command.cpp >> tmp2.cc
        continue;
    fi
    echo "$line"  >> tmp2.cc
done
cp tmp2.cc "../../../operations/gmic/${op_name}.cc"


cp op_config.hh tmp.hh
cat tmp.hh | sed "s/%op_name_uc%/${op_name_uc}/" > tmp2.hh
cp tmp2.hh tmp.hh
cat tmp.hh | sed "s/%op_name%/${op_name}/" > tmp2.hh
cp tmp2.hh tmp.hh
cat tmp.hh | sed "s/%op_name2%/${op_name2}/" > tmp2.hh

cp tmp2.hh tmp.hh
rm -f tmp2.hh
par_def=$(cat par_def.cpp)
echo "$par_def"
nl=$(cat tmp.hh | wc -l)
for i in $(seq 1 $nl); do
    line=$(cat tmp.hh | sed -n ${i}p)
    test=$(echo "$line" | grep '%widgets_def%')
    if [ x"$test" = "x" ]; then
        echo "$line" >> tmp2.hh
    else
        cat widgets_def.cpp >> tmp2.hh
    fi
done
cp tmp2.hh "../../../gui/operations/gmic/${op_name}_config.hh"



test=$(cat "../../../gui/operations/gmic/configs.hh" | grep "${op_name}_config.hh")
if [ x"$test" = "x" ]; then
    echo "#include \"${op_name}_config.hh\"" >> "../../../gui/operations/gmic/configs.hh"
fi


cp op_config.cc tmp.cc
cat tmp.cc | sed "s/%op_name_uc%/${op_name_uc}/" > tmp2.cc
cp tmp2.cc tmp.cc
cat tmp.cc | sed "s/%op_name%/${op_name}/" > tmp2.cc
cp tmp2.cc tmp.cc
cat tmp.cc | sed "s/%op_name2%/${op_name2}/g" > tmp2.cc
cp tmp2.cc tmp.cc
cat tmp.cc | sed "s/%title%/${title}/g" > tmp2.cc

cp tmp2.cc tmp.cc
rm -f tmp2.cc
nl=$(cat tmp.cc | wc -l)
for i in $(seq 1 $nl); do
    line=$(cat tmp.cc | sed -n ${i}p)
    test=$(echo "$line" | grep '%widgets_imp%')
    if [ x"$test" != "x" ]; then
        cat widgets_imp.cpp >> tmp2.cc
        continue;
    fi
    test=$(echo "$line" | grep '%widgets_imp2%')
    if [ x"$test" != "x" ]; then
        cat widgets_imp2.cpp >> tmp2.cc
        continue;
    fi
    test=$(echo "$line" | grep '%command%')
    if [ x"$test" != "x" ]; then
        cat command.cpp >> tmp2.cc
        continue;
    fi
    echo "$line"  >> tmp2.cc
done
cp tmp2.cc "../../../gui/operations/gmic/${op_name}_config.cc"


test=$(cat "../../../operations/gmic/operations.hh" | grep "new_gmic_${op_name}")
if [ x"$test" = "x" ]; then
    rm -f tmp.hh
    nl=$(cat "../../../operations/gmic/operations.hh" | wc -l)
    for i in $(seq 1 $nl); do
        line=$(cat "../../../operations/gmic/operations.hh" | sed -n ${i}p)
        test=$(echo "$line" | grep '//insert new operations here')
        if [ x"$test" != "x" ]; then
            echo "  ProcessorBase* new_gmic_${op_name}();" >> tmp.hh
            echo "  //insert new operations here" >> tmp.hh
            continue;
        fi
        echo "$line" >> tmp.hh
    done
    cp tmp.hh "../../../operations/gmic/operations.hh"
fi


test=$(cat "../../../operations/gmic/new_gmic_operation.cc" | grep "new_gmic_${op_name}")
if [ x"$test" = "x" ]; then
    rm -f tmp.cc
    nl=$(cat "../../../operations/gmic/new_gmic_operation.cc" | wc -l)
    for i in $(seq 1 $nl); do
        line=$(cat "../../../operations/gmic/new_gmic_operation.cc" | sed -n ${i}p)
        test=$(echo "$line" | grep '//insert new operations here')
        if [ x"$test" != "x" ]; then
            echo "  } else if( op_type == \"gmic_${op_name}\" ) {" >> tmp.cc

            echo "    processor = new_gmic_${op_name}();" >> tmp.cc
            echo "    //insert new operations here" >> tmp.cc
            continue;
        fi
        echo "$line" >> tmp.cc
    done
    cp tmp.cc "../../../operations/gmic/new_gmic_operation.cc"
fi


test=$(cat "../../../gui/operations/gmic/new_gmic_operation_config.cc" | grep "Gmic${op_name2}ConfigDialog")
if [ x"$test" = "x" ]; then
    rm -f tmp.cc
    nl=$(cat "../../../gui/operations/gmic/new_gmic_operation_config.cc" | wc -l)
    for i in $(seq 1 $nl); do
        line=$(cat "../../../gui/operations/gmic/new_gmic_operation_config.cc" | sed -n ${i}p)
        test=$(echo "$line" | grep '//insert new operations here')
        if [ x"$test" != "x" ]; then
            echo "  } else if( op_type == \"gmic_${op_name}\" ) {" >> tmp.cc
            echo "    dialog = new PF::Gmic${op_name2}ConfigDialog( current_layer );" >> tmp.cc
            echo "    //insert new operations here" >> tmp.cc
            continue;
        fi
        echo "$line" >> tmp.cc
    done
    cp tmp.cc "../../../gui/operations/gmic/new_gmic_operation_config.cc"
fi


rm -f par_def.cpp widgets_def.cpp par_imp.cpp widgets_imp.cpp par_imp2.cpp widgets_imp2.cpp command.cpp tmp.hh tmp.cc tmp2.hh tmp2.cc

