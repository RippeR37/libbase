# -- Project information -----------------------------------------------------

project = 'libbase'
copyright = '2025, Damian Dyńdo'
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

# -- Setup -------------------------------------------------------------------

import os
import requests

def download_external_rst(app):
    url = 'https://raw.githubusercontent.com/google/glog/refs/tags/v0.7.1/README.rst'
    local_dir = os.path.join(app.srcdir, 'build', 'third_party/glog')
    local_path = os.path.join(local_dir, 'README.rst')

    os.makedirs(local_dir, exist_ok=True)

    if not os.path.exists(local_path):
        print(f"Downloading {url} to {local_path}")
        response = requests.get(url)
        response.raise_for_status()
        with open(local_path, 'w', encoding='utf-8') as f:
            f.write(response.text)

def setup(app):
    app.add_css_file('custom.css')
    app.connect('builder-inited', download_external_rst)
