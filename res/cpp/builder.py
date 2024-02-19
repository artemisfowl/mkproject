#!/usr/bin/env python3

from pathlib import Path
from os import sep, getcwd, system
from sys import exit
from json import dump, load

BUILD_ICON = "\U0001F528"
ERROR_ICON = "\U0001F626"
PASS_ICON = "\U0001F60E"
FILE_ICON = "\U0001F4C4"
EDIT_ICON = "\U0001F589"
GEAR_ICON = "\U00002699"

def update_build_info(major: int = 0, minor: int = 0, build_number: int = 1) -> None:
	with open(f"{getcwd()}{sep}inc/buildinfo.h", "w") as binfo_file:
		# write the header guard first
		binfo_file.write("#ifndef BUILDINFO_H\n")
		binfo_file.write("#define BUILDINFO_H\n\n")

		# add the details of the build information
		print(f"{EDIT_ICON} : Updating major value : {major}")
		binfo_file.write("#ifndef MAJOR\n")
		binfo_file.write(f"\t#define MAJOR {major}\n")
		binfo_file.write("#endif\n\n")
		print(f"{EDIT_ICON} : Updating minor value : {minor}")
		binfo_file.write("#ifndef MINOR\n")
		binfo_file.write(f"\t#define MINOR {minor}\n")
		binfo_file.write("#endif\n\n")
		print(f"{EDIT_ICON} : Updating build number value : {build_number}")
		binfo_file.write("#ifndef BUILD_NUMBER\n")
		binfo_file.write(f"\t#define BUILD_NUMBER {build_number}\n")
		binfo_file.write("#endif\n\n")

		# write the header guard completion
		binfo_file.write("#endif")


def store_version_info(major: int, minor: int, build_number: int) -> None:
	data = {"major": major, "minor": minor, "build_number": build_number}
	with open(f"{getcwd()}{sep}.version_info.json", "w") as version_info_file:
		dump(data, version_info_file)


def read_version_info() -> dict:
	with open(f"{getcwd()}{sep}.version_info.json") as version_info_file:
		data = load(version_info_file)

	return data

def make_project() -> None:
	system("make")


def main() -> None:
	major, minor, build_number = 0, 0, 1
	print(f"{GEAR_ICON} : Building project - {str(Path(__file__).resolve()).split(sep)[-2]}\n")

	if not Path(f"{getcwd()}{sep}Makefile").exists():
		print(f"{ERROR_ICON} : Makefile is not present in this directory")
		print("Please place this file in the same directory where 'Makefile' is present")
		print("Exiting...\n")
		exit(-1)
	else:
		print(f"{PASS_ICON} : Makefile found\n")

		# check if the header file is present or not for capturing version information
		if not Path(f"{getcwd()}{sep}inc{sep}buildinfo.h").exists():
			print(f"{FILE_ICON} : buildinfo header file is not present - creating it")
			update_build_info()
			store_version_info(major=major, minor=minor, build_number=build_number)

			print(f"\n{BUILD_ICON} : Building project ...\n")
			make_project()
		else:
			# if the file is present, read the file and update the values before incrementing the build number
			print(f"{FILE_ICON} : buildinfo header file is present - updating it")
			print(f"{GEAR_ICON} : Updating build versions")

			data = read_version_info()
			major = data["major"]
			minor = data["minor"]
			build_number = data["build_number"]

			if build_number >= 2600:
				build_number = 1
				if minor >= 100:
					major += 1
					minor = 0
				else:
					minor += 1
			else:
				build_number += 1

			update_build_info(major=major, minor=minor, build_number=build_number)
			store_version_info(major=major, minor=minor, build_number=build_number)

			print(f"\n{BUILD_ICON} : Building project ...\n")
			make_project()
		print()

	exit(0)

if __name__ == "__main__":
	main()
