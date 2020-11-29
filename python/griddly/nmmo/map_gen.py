from pdb import set_trace as T
import numpy as np
import yaml
np.set_printoptions(threshold=5000, linewidth=200)

class MapGen():
    MAP_WIDTH = 50
    def __init__(self):
        self.probs = {
                'grass':  0.60,
                'water':  0.25,
                'shrubs': 0.00,
                'rock':   0.15,
                'lava':   0.00,
                }
        self.chars = {
                'grass': '.'
                }

    def get_init_tiles(self, yaml_path):
        # Using a template to generate the runtime file allows for preservation of comments and structure. And possibly other tricks... (evolution of game entities and mechanics)
        yaml_template_path = yaml_path.strip('.yaml') + '_template.yaml'
        init_tiles = [self.chars['grass']]
        probs = [self.probs['grass']]
        tile_types = list(self.probs.keys())
        with open(yaml_template_path) as f:
            contents = yaml.load(f, Loader=yaml.FullLoader)
            self.utf_enc = 'U' + str(len(str(contents['Environment']['Player']['Count'])) + 1)
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
            self.n_players = contents['Environment']['Player']['Count']
        assert hasattr(self, 'player_char')
        # Add a placeholder level so that we can make the env from yaml (this will be overwritten during reset)
        level_string = self.gen_map(init_tiles, probs)
        contents['Environment']['Levels'] = [level_string]
        with open(yaml_path, 'w') as f:
            yaml.dump(contents, f)

        return init_tiles, probs

    def gen_map(self, init_tiles, probs):
        # max 3 character string in each tile
        # need to take into account column of newlines
        level_string = np.random.choice(init_tiles, size=(MapGen.MAP_WIDTH, MapGen.MAP_WIDTH+1), p=probs).astype(self.utf_enc)
        idxs = np.where(level_string[1:-1, 1:-2])
        idxs = np.array(list(zip(idxs[0], idxs[1]))) + 1
        ixs = np.random.choice(len(idxs), self.n_players, replace=False)
        coords = idxs[ixs]
        border_tile = 'water'
        level_string[0, :] = self.chars[border_tile]
        level_string[-1, :] = self.chars[border_tile]
        level_string[:, 0] = self.chars[border_tile]
        level_string[:, -2] = self.chars[border_tile]
        for j, coord in enumerate(coords):
            level_string[coord[0], coord[1]] = self.player_char + str(j+1)
#       level_string[coords[:, 0], coords[:, 1]] = self.player_char

        level_string[:, -1] = '\n'
        level_string = ' '.join(s for s in level_string.reshape(-1))
        return level_string

