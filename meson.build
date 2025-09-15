project('CResult', 'c',
  version : '0.1.0',
  default_options : ['c_std=c11', 'warning_level=2'])

# Ajoute le répertoire racine au chemin de recherche des en-têtes
# pour que les exemples puissent trouver "maybe.h"
inc = include_directories('.')

# Déclare les quatre exécutables d'exemples
executable('result_example', 'examples/result.c',
  include_directories : inc)

executable('optional_example', 'examples/optional.c',
  include_directories : inc)

executable('optional_in_result_example', 'examples/optional_in_result.c',
  include_directories : inc)

executable('chaining_example', 'examples/chaining.c',
  include_directories : inc)
