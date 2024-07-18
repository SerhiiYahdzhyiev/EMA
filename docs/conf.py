import os

from clang.cindex import Config

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

if 'CLANG_LIBRARY_FILE' in os.environ:
    Config.set_library_file(os.environ['CLANG_LIBRARY_FILE'])

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'EMA'
copyright = '2024, PERFACCT GmbH'
author = 'Danny Puhan, Johannes Spazier'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx_c_autodoc',
    'sphinx_c_autodoc.napoleon',
]
templates_path = ['_templates']
exclude_patterns = ['_build']
autodoc_default_options = {
    'members': True,
}

# -- Configuration for sphinx-c-autodoc --------------------------------------
# https://sphinx-c-autodoc.readthedocs.io/en/latest/configuration.html

c_autodoc_roots = ['../EMA']

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'pyramid'
html_show_sourcelink = False
