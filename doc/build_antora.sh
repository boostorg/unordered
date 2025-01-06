#!/bin/bash

set -ex

npm ci
npx antora unordered-playbook.yml
