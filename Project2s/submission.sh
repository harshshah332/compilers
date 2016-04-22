#!/bin/bash

name="HarshShah_MomoSavier"
pid="A12396344_A11230603"
email="hmshah@ucsd.edu_msavier@ucsd.edu"

if [[ $name == "" ]]
then 
	echo "Edit this script to enter your name and partners name above"
	exit
fi

if [[ $pid == "" ]]
then
        echo "Edit this script to enter your PID and partners PID above"
        exit
fi

if [[ $email == "" ]]
then
        echo "Edit this script to enter one email where you can be reached easily"
        exit
fi


if [[ $pid == [A][0-9]* || $pid == [A][0-9]*[_][A][0-9]* || $pid == [A][0-9]*[_][A][0-9]*[_][A][0-9]* ]]
then 
	rm -rf $pid.zip
	rm -rf $pid

	mkdir $pid

	cp ast_decl.cc $pid/
	cp ast_decl.h $pid/
	cp ast_expr.cc $pid/
	cp ast_expr.h $pid/
	cp ast_stmt.cc $pid/
	cp ast_stmt.h $pid/
	cp ast_type.cc $pid/
	cp ast_type.h $pid/
	cp ast.cc $pid/
	cp ast.h $pid/
	cp errors.cc $pid/
	cp errors.h $pid/
	cp list.h $pid/
	cp location.h $pid/
	cp main.cc $pid/
	cp parser.h $pid/
	cp parser.y $pid/
	cp scanner.h $pid/
	cp scanner.l $pid/
	cp utility.cc $pid/
	cp utility.h $pid/
	cp Makefile $pid/
	cp project-description.txt $pid/

	zip -r $pid.zip $pid/*
else 
        echo "The pid provided does not match the criteria on piazza"
        exit
fi
 
