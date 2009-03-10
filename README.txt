The Full Node Version is a drupal module, that allows for a full site preview.
Each page is version so that you can have one version Live and a different
version In Testing.  If allows for all aspects of the node to be different in
this two location (They are infact different nodes). It also allows for all of
the change that are In Testing to be moved to Live.


INSTALLATION AND USE

There are a few things that one needs to do and keep in mind when setting up
the module.  Enable FNV on content types

By default FNV is not active on any nodes. One must turn it on by Node type, on
the Full Node Verison settings page.  No administer nodes perm

FNV uses the node access model to control which content is accesable, the
permission "administer nodes" by pass the model, so anyone with it will see all
version! So users can not have this permission. The main problem this causes is
one no longer has access to the Content page (where one can see all nodes). I
am hoping to add a page like this will lesser perms. Also A view can be used.
Sites and Status

There are two version of a node to which a user has access. Current and Draft.
A node starts off as Draft and become Current when it is Pushed to Live. After
this if it is edited a new version is create, this verison has a Draft status.

So There are three "sites" when we have full Node version:

    * Live: by default this is what anonymous users see,
          o it show all Current version that have a status of Live
    * Working: by default this is what one sees when they are authenticated
          o it show all Draft Versions
          o it there is no Draft Version then it show the Current Version
    * Testing: this is the site seen when one enters testing mode
          o show either Current or Draft, or neither depending on which 
            has a status of testing



ON NODE OPTIONS

On each node is a new tab that allows a user to change which version is on each
site.  Also on node edit one can change the status of the Draft Version Review
Page

The review page list all nodes with a Draft Version and any other nodes whose
testing status is different from the Live status.

    * It shows the differences between the Live and Test Site
    * It lets you change the status of Version
    * This page has a Publish testing option which makes the Live site have the
      same verisons as the Test Site

