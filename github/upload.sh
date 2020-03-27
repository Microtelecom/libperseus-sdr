# https://stackoverflow.com/questions/30066239/cannot-upload-github-release-asset-through-api
# for a comprehensive description of github API see
# https://developer.github.com/v3/repos/releases/#list-assets-for-a-release

# how to get all releases
# https://api.github.com/repos/Microtelecom/libperseus-sdr/releases
# note: the first one listed is the last one added
#

# how to get data about specific release
# 
# https://api.github.com/repos/Microtelecom/libperseus-sdr/releases/tags/v0.8.2
#{
#  "url": "https://api.github.com/repos/Microtelecom/libperseus-sdr/releases/24542383",
#  "assets_url": "https://api.github.com/repos/Microtelecom/libperseus-sdr/releases/24542383/assets",
#  "upload_url": "https://uploads.github.com/repos/Microtelecom/libperseus-sdr/releases/24542383/assets{?name,label}",
#  "html_url": "https://github.com/Microtelecom/libperseus-sdr/releases/tag/v0.8.2",
#  "id": 24542383,
#  "node_id": "MDc6UmVsZWFzZTI0NTQyMzgz",
#  "tag_name": "v0.8.2",
#  "target_commitish": "master",
#  "name": "Integration in openwebsdr; remote build in GitHub",
#  "draft": false,
#  "author": {
#    "login": "amontefusco",
#    "id": 244977,
#    "node_id": "MDQ6VXNlcjI0NDk3Nw==",
#    "avatar_url": "https://avatars3.githubusercontent.com/u/244977?v=4",
#    "gravatar_id": "",
#    "url": "https://api.github.com/users/amontefusco",
#    "html_url": "https://github.com/amontefusco",
#    "followers_url": "https://api.github.com/users/amontefusco/followers",
#    "following_url": "https://api.github.com/users/amontefusco/following{/other_user}",
#    "gists_url": "https://api.github.com/users/amontefusco/gists{/gist_id}",
#    "starred_url": "https://api.github.com/users/amontefusco/starred{/owner}{/repo}",
#    "subscriptions_url": "https://api.github.com/users/amontefusco/subscriptions",
#    "organizations_url": "https://api.github.com/users/amontefusco/orgs",
#    "repos_url": "https://api.github.com/users/amontefusco/repos",
#    "events_url": "https://api.github.com/users/amontefusco/events{/privacy}",
#    "received_events_url": "https://api.github.com/users/amontefusco/received_events",
#    "type": "User",
#    "site_admin": false
#  },
#  "prerelease": false,
#  "created_at": "2020-03-15T21:41:05Z",
#  "published_at": "2020-03-15T21:48:50Z",
#  "assets": [
#
#  ],
#  "tarball_url": "https://api.github.com/repos/Microtelecom/libperseus-sdr/tarball/v0.8.2",
#  "zipball_url": "https://api.github.com/repos/Microtelecom/libperseus-sdr/zipball/v0.8.2",
#  "body": "More command line options where added to perseustest in order to make all hardware features of Perseus exploitable from the command line.\r\nA first attempt of GitHub remote build was configured."
#}
#

#
# to load an assets (i.e. a tar.gz file) use the following script 
# the RELEASE variable is taken from id field from the release dump above
# 
FILE="libperseus_sdr-0.8.2.tar.gz"
RELEASE="24542383"
GITHUB_TOKEN="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

curl \
    -H "Authorization: token $GITHUB_TOKEN" \
    -H "Content-Type: $(file -b --mime-type $FILE)" \
    --data-binary @$FILE \
    "https://uploads.github.com/repos/Microtelecom/libperseus-sdr/releases/$RELEASE/assets?name=$(basename $FILE)"
