.PHONY: tests compile_tests clean check_updates update

tests: compile_tests

compile_tests:
	bash test_all_platforms.sh

update:
	python appli/iotlab/lib/scripts/generate_uid_dict.py

check_updates: update
	@if [[ $$(git status --porcelain --untracked-files=no) != '' ]]; \
	then \
		echo "ERROR: Following files are outdated:" ; \
		git status --porcelain --untracked-files=no ; \
		exit 1; \
	fi

clean:
	rm -rf build.*
