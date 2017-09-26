# embedded-software
Embedded Software Labs

This is our official place of all things Embedded. Will most likely have to push changes here to the subversion repo once we've finalised but before then, we can do version controlled changes here.

So to start:

- Go to your terminal and clone the repository (you can use github desktop or just do it via terminal since it's easier). Copy the link to clone.

- Then open the cloned repo with any editor of your choice (Kinetis is the ideal one for this lab or Atom / any editor if you're accessing this from home.

- BEFORE YOU MAKE ANY CHANGES, BRANCH OFF MASTER FIRST. To do that simply go to your terminal and do git checkout -b branch-name. This will automatically create a branch and switch to that branch. (branch-name can be whatever you want but make sure it's meaningful i.e. git checkout -b UART-Poll or something).

- We want to push the branch to the remote first so do git push --set-upstream origin branch-name. This will push the branch to the remote with tracking and create a new pull request.

- Switch to github browser and you should see the pull request made. Create it and you're good to go!

- You can now make changes to the IDE of your choice.

- Do a git status to list changes you've made and add the files you modified (usually in red) by doing a git add folder/path (it'll be shown in the modified section when you do a git status)

- To commit those changes to the branch simply do git commit -m "Your message to do with what you changed in here."

- Then git push. Check in the github repo web page that your changes have been pushed.

- Once you're finished with your changes for that branch simply merge to master via the github web interface.

NOTE: any subsequent branches you make after this, you want to checkout master first then do a git pull before doing a git checkout -b branch-name to make sure that branch has the latest changes.
