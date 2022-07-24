# -- Project information -----------------------------------------------------

project = 'libbase'
copyright = '2022, Damian Dyńdo'
author = 'Damian Dyńdo'

# Breathe extension
breathe_projects = {
    "libbase": "build/xml"
}
breathe_default_project = "libbase"


# -- General configuration ---------------------------------------------------

import textwrap

extensions = [
  "breathe",
  "exhale",
  "sphinx.ext.autosectionlabel",
  "sphinx.ext.todo",
  "sphinx_multiversion",
]

# Exhale extension
exhale_args = {
  "containmentFolder":     "./build/api",
  "rootFileName":          "index.rst",
  "rootFileTitle":         "C++ API reference",
  "doxygenStripFromPath":  "../src",
  "verboseBuild":          True,
  "createTreeView":        True,
  "exhaleExecutesDoxygen": True,
  "exhaleDoxygenStdin":    textwrap.dedent('''
    INPUT                   = ../src
    GENERATE_XML            = YES
    GENERATE_HTML           = NO
    GENERATE_LATEX          = NO
    XML_PROGRAMLISTING      = NO
    VERBATIM_HEADERS        = NO
    CLANG_ASSISTED_PARSING  = YES
    EXCLUDE_PATTERNS        = */CMakeLists.txt,*.cc
    EXCLUDE_SYMBOLS         = *detail*,*.cc
  ''')
}

smv_remote_whitelist = r'^(origin)$'
smv_branch_whitelist = r'^(master|develop).*$'
smv_tag_whitelist = r'^v(\d+\.\d+|\d+\.\d+\.\d+)$'

templates_path = ['_templates']

exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

html_theme = 'sphinx_rtd_theme'

html_theme_options = {
    'collapse_navigation': False,
    'navigation_depth': 3,
}

html_context = {
    'display_github': True,
    'github_user': 'RippeR37',
    'github_repo': 'libbase',
    'github_version': 'master',
    'conf_py_path': '/docs/',
}

html_show_sourcelink = True

html_static_path = ['_static']

primary_domain = 'cpp'
highlight_language = 'cpp'

todo_include_todos = True
