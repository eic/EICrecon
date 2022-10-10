# Documentation website

## Development

The site is served by [Docsify.js](https://docsify.js.org/). It is a fully dynamic website
that is driven and configured by index.html, in the [EICrecon/docs](https://github.com/eic/EICrecon/tree/main/docs) folder.
Depending on user requests (links) it dynamically loads Markdown documents and show them to user.

For development on can use any development server that can serve the index.html. 
While there is no need to use a dedicated JS environment to develop the documentation, 
docsify itself provide a handful tool that could be used to run the site on a local machine. 

```bash
npm install -g docsify  # once to install docsify

docsify serve docs      # to run a server with the document
```


## Markdown format

Docsify follows GFM (GitHub Flavoured Markdown) format and has a couple of handy extensions such as:

1. [Image resizing](https://docsify.js.org/#/helpers?id=resizing): 
   ```markdown
    ![logo](https://docsify.js.org/_media/icon.svg ':size=50x100')
   ```
2.[Embed a page or a part of a page (!)](https://docsify.js.org/#/embed-files) in another page: 
   ```markdown
   [filename](_media/example.md ':include')
   ```

[Docsify full documentation](https://docsify.js.org/#/?id=docsify)
