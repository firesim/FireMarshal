#!/usr/bin/env python3

from pathlib import Path
import os
from github import Github
import time
import zipfile

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
fm_dir = script_dir.parent

GH_REPO = 'firemarshal-public-br-images'
GH_ORG = 'firesim'
URL_PREFIX = f"https://raw.githubusercontent.com/{GH_ORG}/{GH_REPO}"


# taken from https://stackoverflow.com/questions/63427607/python-upload-files-directly-to-github-using-pygithub
# IMPORTANT: only works for binary files! (i.e. tar.gz or zip files)
def upload_binary_file(local_file_path, gh_file_path):
    print(f":DEBUG: Attempting to upload {local_file_path} to {gh_file_path}")

    g = Github(os.environ['PERSONAL_ACCESS_TOKEN'])

    repo = g.get_repo(f'{GH_ORG}/{GH_REPO}')
    all_files = []
    contents = repo.get_contents("")
    while contents:
        file_content = contents.pop(0)
        if file_content.type == "dir":
            contents.extend(repo.get_contents(file_content.path))
        else:
            file = file_content
            all_files.append(str(file).replace('ContentFile(path="', '').replace('")', ''))

    with open(local_file_path, 'rb') as file:
        content = file.read()

    tries = 10
    delay = 15
    msg = f"Committing files from {os.environ['GITHUB_SHA']}"
    upload_branch = 'main'
    r = None

    # Upload to github
    git_file = gh_file_path
    if git_file in all_files:
        contents = repo.get_contents(git_file)
        for n in range(tries):
            try:
                r = repo.update_file(contents.path, msg, content, contents.sha, branch=upload_branch)
                break
            except Exception as e:
                print(f"Got exception: {e}")
                time.sleep(delay)
        assert r is not None, f"Unable to poll 'update_file' API {tries} times"
        print(f"Updated: {git_file}")
    else:
        for n in range(tries):
            try:
                r = repo.create_file(git_file, msg, content, branch=upload_branch)
                break
            except Exception as e:
                print(f"Got exception: {e}")
                time.sleep(delay)
        assert r is not None, f"Unable to poll 'create_file' API {tries} times"
        print(f"Created: {git_file}")

    return r['commit'].sha


def make_relative(path):
    path_str = str(path)
    return path_str.replace(f"{fm_dir}/", "")


# only caches firechip board br images
for e in (fm_dir / 'images' / 'firechip').iterdir():
    if not e.is_file():
        for ie in e.iterdir():
            if ie.is_file() and ie.suffix == '.img' and "br." in ie.name:
                ie_zip = str(ie) + ".zip"
                with zipfile.ZipFile(ie_zip, 'w', zipfile.ZIP_BZIP2, compresslevel=9) as zip_ref:
                    zip_ref.write(ie, ie.name)
                upload_binary_file(ie_zip, make_relative(ie_zip))
                os.remove(ie_zip)
