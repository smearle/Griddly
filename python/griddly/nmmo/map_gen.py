from pdb import set_trace as T
import numpy as np
import yaml
np.set_printoptions(threshold=5000, linewidth=200)

def replace_vars(yaml_contents, var_dict):
    if isinstance(yaml_contents, dict):
        for key, val in yaml_contents.items():
            new_entry = replace_vars(val, var_dict)
            if new_entry is not None:
                yaml_contents[key] = new_entry
    elif isinstance(yaml_contents, list):
        for el in yaml_contents:
            replace_vars(el, var_dict)
    elif isinstance(yaml_contents, str):
        if yaml_contents in var_dict:
            return var_dict[yaml_contents]
        else:
            return None
    else:
        pass
       #raise Exception('Unexpected type, {}, while parsing yaml.'.format(
       #    type(yaml_contents)))

class MapGen():
    MAP_WIDTH = 50
    N_PLAYERS = 25
    INIT_DELAY = 10 # how long to wait before decrementing hunger & thirst
    INIT_HEALTH = 10
    INIT_THIRST = 10
    INIT_HUNGER = 10
    SHRUB_RESPAWN = 15

    VAR_DICT = {
            '${_init_delay}':    INIT_DELAY * N_PLAYERS,
            '${_delay}':         1 * N_PLAYERS,
            '${_init_health}':   INIT_HEALTH,
            '${_init_hunger}':   INIT_HUNGER,
            '${_init_thirst}':   INIT_THIRST,
            '${_shrub_respawn}': SHRUB_RESPAWN * N_PLAYERS,
            }

    def __init__(self):
        self.probs = {
                'grass':  0.60,
                'water':  0.10,
                'shrub':  0.10,
                'rock':   0.10,
                'lava':   0.10,
                }
        self.chars = {
                'grass': '.'
                }
        self.border_tile = 'water'

    def get_init_tiles(self, yaml_path):
        # Using a template to generate the runtime file allows for preservation of comments and structure. And possibly other tricks... (evolution of game entities and mechanics)
        yaml_template_path = yaml_path.strip('.yaml') + '_template.yaml'
        init_tiles = [self.chars['grass']]
        probs = [self.probs['grass']]
        tile_types = list(self.probs.keys())
        with open(yaml_template_path) as f:
            contents = yaml.load(f, Loader=yaml.FullLoader)
        self.utf_enc = 'U' + str(len(str(MapGen.N_PLAYERS)) + 1)
        objects = contents['Objects']
        for obj in objects:
            obj_name = obj['Name']
            if obj_name in tile_types:
                char = obj['MapCharacter']
                init_tiles.append(char)
                probs.append(self.probs[obj_name])
                self.chars[obj_name] = char
            if obj['Name'] == 'gnome':
                self.player_char = obj['MapCharacter']#+ '1'
        assert hasattr(self, 'player_char')
        # Add a placeholder level so that we can make the env from yaml (this will be overwritten during reset)
        level_string = self.gen_map(init_tiles, probs)
        contents['Environment']['Levels'] = [level_string] # placeholder map
        contents['Environment']['Player']['Count'] = MapGen.N_PLAYERS # set num players
        #HACK: scale delays to num players
        replace_vars(contents, MapGen.VAR_DICT)
        with open(yaml_path, 'w') as f:
            yaml.safe_dump(contents, f, default_style=None, default_flow_style=False)

        return init_tiles, probs

    def gen_map(self, init_tiles, probs):
        # max 3 character string in each tile
        # need to take into account column of newlines
        level_string = np.random.choice(init_tiles, size=(MapGen.MAP_WIDTH, MapGen.MAP_WIDTH+1), p=probs).astype(self.utf_enc)
        idxs = np.where(level_string[1:-1, 1:-2])
        idxs = np.array(list(zip(idxs[0], idxs[1]))) + 1
        ixs = np.random.choice(len(idxs), MapGen.N_PLAYERS, replace=False)
        coords = idxs[ixs]
        border_tile = self.border_tile
        level_string[0, :] = self.chars[border_tile]
        level_string[-1, :] = self.chars[border_tile]
        level_string[:, 0] = self.chars[border_tile]
        level_string[:, -2] = self.chars[border_tile]
        for j, coord in enumerate(coords):
            level_string[coord[0], coord[1]] = self.player_char + str(j+1)
#       level_string[coords[:, 0], coords[:, 1]] = self.player_char

        level_string[:, -1] = '\n'
        level_string = ' '.join(s for s in level_string.reshape(-1))
        return level_string[:-2]

