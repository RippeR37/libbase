Weak pointers
=============

``libbase`` implements Chromium's weak pointers [#chr_weak_ptr]_ to allow
writing safer code. This module consists of two classes:

* |WeakPtr|
     This class represents a weak pointer to an object of type ``T`` which can
     be used to check if this object is still alive (or if the
     weak pointer itself is still valid, to be precise [#valid_expl]_) and if it
     is, to access it in a safe manner.

* |WeakPtrFactory|
     This class provides a functionality of creation of |WeakPtr| objects and
     managing whether they are still valid and can be used.

     .. note::

        A single instance of |WeakPtrFactory| refers only to one object of type
        ``T`` throughout its lifetime.

The notable difference between the |WeakPtr| class and the standard
``std::weak_ptr<T>`` class is that |WeakPtr| objects do not extend the lifetime
of the pointed-to object and therefore can be used to point to objects allocated
on the stack. The user is required to check if the object is still accessible
(with :func:`base::WeakPtr::operator bool`) before accessing the pointed-to
object - which can be done with any of these functions:

* :func:`base::WeakPtr::operator->`,
* :func:`base::WeakPtr::operator*`, or
* :func:`base::WeakPtr::Get`.

A |WeakPtr| objects are created from a |WeakPtrFactory| object. When
constructing the factory, it requires a pointer to the object to which all the
|WeakPtr| objects created from it will point to. Then, all created |WeakPtr|
objects will be considered valid until either the factory is destroyed or a call
to the :func:`base::WeakPtrFactory::InvalidateWeakPtrs` member function is made.

.. note::

   It is still possible to create new and valid |WeakPtr| objects from
   |WeakPtrFactory| on which the
   :func:`base::WeakPtrFactory::InvalidateWeakPtrs` member function was called.
   The invalidation affects only existing weak pointers.


.. important::

   The |WeakPtrFactory| is often the last member in the class it generates weak
   pointers to. This ensures that all existing weak pointers will be invalidated
   before any other member variable from that class will be destroyed.

Once a first weak pointer from a given factory is dereferenced (tested or used
to access the pointed-to object), both the factory and all the existing weak
pointers created by it become bound to the sequence on which it happend and from
that point in time can only be dereferenced or invalidated on that sequence.

.. caution::

   While it is perfectly safe to copy and pass |WeakPtr| objects across
   different :doc:`threads and sequences <threads>`, they can be used only on
   the sequence on which they were bound!

   This is required to ensure that the pointed-to object is not destroyed
   (on a different thread) between checking if it is still alive and actually
   accessing it.

.. caution::

   If all weak pointers created by a given factory would be destroyed, the
   factory automatically unbinds itself from the bounded sequence and newly
   created |WeakPtr| can be used on different sequences.

.. admonition:: Example - |WeakPtr| and |WeakPtrFactory| usage
   :class: admonition-example-code

   .. code-block:: cpp
      :linenos:

      #include <iostream>

      #include "base/memory/weak_ptr.h"

      class Foo {
       public:
        void Bar() {
          std::cout << "Hello World!" << std::endl;
        }

        base::WeakPtr<Foo> GetWeakPtr() const {
          return weak_factory_.GetWeakPtr();
        }

       private:
        base::WeakPtrFactory<Foo> weak_factory_{this};
      };

      int main() {
        std::unique_ptr<Foo> foo = std::make_unique<Foo>();
        base::WeakPtr<Foo> weak_foo = foo->GetWeakPtr();

        CHECK(weak_foo);
        weak_foo->Bar();  // prints "Hello World!"

        foo.reset();
        CHECK(!weak_foo);
        // must NOT use `weak_foo` to try to access the pointed-to object anymore

        return 0;
      }

.. seealso::

   Check :doc:`callbacks` page to see how |WeakPtr| objects can be used to
   write safer code with callbacks or to create cancelable callbacks.


.. Footnotes:

.. [#chr_weak_ptr] https://chromium.googlesource.com/chromium/src/+/HEAD/base/memory/weak_ptr.h

.. [#valid_expl] It is possible to invalidate |WeakPtr| before the object is destroyed.
   This can be useful when you need to prevent access to the object from *old*
   users or if you need to invalidate any existing callbacks.


.. Aliases:

.. |WeakPtr| replace:: :class:`base::WeakPtr\<T> <base::WeakPtr>`
.. |WeakPtrFactory| replace:: :class:`base::WeakPtrFactory\<T> <base::WeakPtrFactory>`
