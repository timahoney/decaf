# Decaf

**Decaf** is a modification of WebKit that runs Ruby in the browser. You can access the DOM, use the inspector, and do most other things that JavaScript can do. For now, it only builds on the Mac. You can [download a binary](http://trydecaf.org/latest) or you can [build it yourself](http://www.webkit.org/building/build.html).

```html
<script type='text/ruby'>
    window.onload do
        introduction = document.create_element('p')
        introduction.inner_text = 'Hello, world!'
        document.body.append_child(introduction)
    end
</script>
```

## Using Ruby in the Browser

The same DOM API used by JavaScript is available in Ruby. If you're not familiar with JavaScript and the DOM, then check out these wonderful resources:

- __Mozilla's [DOM Reference](https://developer.mozilla.org/en-US/docs/DOM) and [Gecko DOM Reference](https://developer.mozilla.org/en-US/docs/Gecko_DOM_Reference)__
- __[W3C DOM Reference](http://www.w3.org/TR/domcore)__
- __[Web Platform Documentation](http://www.webplatform.org)__

### Differences from JavaScript

There are a few major differences between the DOM API in JavaScript and Ruby.

- In Ruby, methods and attributes are specified in `underscore_case` instead of `camelCase`.
- The `window` variable is accessible from only the top-most scope. Elsewhere you can use the global `$window`.
- Ruby accepts `Procs` and blocks as callbacks and listeners. For example:

```ruby
# Ruby with implicit blocks
$window.set_timeout(1000) { console.log('Hello!') }
$window.onload do |event|
  console.log('The window loaded.')
end

# Ruby with explicit Procs
$window.set_timeout(Proc.new { console.log('Hello!') }, 1000)
$window.onload = Proc.new do |event|
  console.log('The window loaded.')
end
```

## Warning
This is a work in progress. Please report any issues you find.
