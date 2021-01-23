#!/bin/bash

function confirm {
  echo -n "Are you sure to delete ALL releases, ALL remote tags and ALL local tags? "
  read -r
  echo
}

# https://wiki.archlinux.org/index.php/Proxy_server
function set_proxy {
  for envar in \
    http_proxy https_proxy all_proxy \
    HTTP_PROXY HTTPS_PROXY ALL_PROXY \
  ; do
    # echo "$envar"
    export $envar="http://127.0.0.1:8080/"
  done
}

function delete_all_releases {
  # gh release list | cut --delimiter=$'\t' --fields=1;echo
  # gh release list | cut --delimiter=$'\t' --fields=2;echo
  # gh release list | cut --delimiter=$'\t' --fields=3;echo
  # gh release list | cut --delimiter=$'\t' --fields=1 | {
  #   while read -r title; do
  #     echo "$title"
  #   done
  # }
  local tag
  for tag in $(gh release list | cut --delimiter=$'\t' --fields=3); do
    # gh release view "$tag"
    gh release delete "$tag"
  done
}

# https://stackoverflow.com/questions/44702757
function delete_all_local_and_remote_tags {

  # https://devconnected.com/how-to-delete-local-and-remote-tags-on-git/
  # Delete a remote Git tag
  # proxychains /usr/bin/git push --delete origin <tag>
  # proxychains /usr/bin/git push origin :refs/tags/<tag>
  # printf "prefix-%s " a b c d

  echo "1. Delete all local tags"
  git tag --list | xargs git tag --delete
  echo

  echo "2. Fetch all remote tags"
  proxychains /usr/bin/git fetch --tags --verbose
  # proxychains /usr/bin/git fetch --force --tags --verbose
  echo

  echo "3. Delete all remote tags"
  tags="$(git tag --list)"
  if [ -n "$tags" ]; then
    printf " :refs/tags/%s " "$tags" | xargs proxychains /usr/bin/git push --verbose origin
  fi
  echo

  echo "4. Delete all local tags (again)"
  git tag --list | xargs git tag --delete
  echo

}

# https://github.com/Un1Gfn-nt/ntexec/releases/new
function tag_and_release {
  pkgver="r$(git rev-list --count HEAD).$(git rev-parse --short HEAD)"
  notes="$(git rev-parse HEAD) $(git show --format=%s --no-patch HEAD)"
  # echo "$pkgver ($pkgver)"
  # echo "$description"
  git tag "$pkgver"
  # for i in "$@"; do echo "#$i#"; done
  gh release create "$pkgver" "$@" --notes "$notes" --prerelease --title "$pkgver"
}

echo
confirm
set_proxy
delete_all_releases
delete_all_local_and_remote_tags
tag_and_release "$@"
