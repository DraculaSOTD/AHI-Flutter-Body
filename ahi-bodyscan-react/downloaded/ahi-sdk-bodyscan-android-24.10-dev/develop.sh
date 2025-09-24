#!/usr/bin/env bash

export AHI_RELEASE_AWS_ACCESS_KEY=AKIASR46NPIHVCEL63MH
export AHI_RELEASE_AWS_SECRET_KEY=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
export AHI_DEV_AWS_ACCESS_KEY=AKIASR46NPIH2OITSGX4
export AHI_DEV_AWS_SECRET_KEY=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

export JAVA_HOME=/Applications/Android\ Studio.app/Contents/jre/Contents/Home/

git branch --show-current | sed -E "s/\/.+/\.0/" > VERSION.txt
git branch --show-current | sed -E "s/\/.+//" > VERSION.MAJOR.txt
perl -p -i -e 's/\R//g;' VERSION.txt
perl -p -i -e 's/\R//g;' VERSION.MAJOR.txt

# ./gradlew --refresh-dependencies
open -a /Applications/Android\ Studio.app/ .
