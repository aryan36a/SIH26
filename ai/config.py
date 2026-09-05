from pathlib import Path


# ------------------------------------------------------------
# Dataset
# ------------------------------------------------------------

DATASET_ROOT = Path("data/semantic-kitti")

TRAIN_SEQUENCES = [
    "00",
    "01",
    "02",
    "03",
    "04",
    "05",
    "06",
    "07",
    "09",
    "10",
]

VAL_SEQUENCES = [
    "08",
]


# ------------------------------------------------------------
# Features
# ------------------------------------------------------------

NUM_FEATURES = 5

FEATURE_NAMES = [
    "x",
    "y",
    "z",
    "intensity",
    "range",
]


# ------------------------------------------------------------
# SemanticKITTI learning classes
# ------------------------------------------------------------

NUM_CLASSES = 19

# SemanticKITTI raw semantic IDs -> learning IDs.
# 255 means "ignore".
LEARNING_MAP = {
    0: 255,

    1: 255,      # outlier

    10: 0,       # car
    11: 1,       # bicycle
    13: 2,       # bus
    15: 3,       # motorcycle
    16: 4,       # on-rails
    18: 5,       # truck
    20: 6,       # other-vehicle
    30: 7,       # person
    31: 8,       # bicyclist
    32: 9,       # motorcyclist

    40: 10,      # road
    44: 11,      # parking
    48: 12,      # sidewalk
    49: 13,      # other-ground
    50: 14,      # building
    51: 15,      # fence
    52: 16,      # other-structure
    60: 17,      # lane-marking

    70: 18,      # vegetation

    71: 255,     # trunk
    72: 255,     # terrain
    80: 255,     # pole
    81: 255,     # traffic-sign
    99: 255,     # other-object
}


IGNORE_INDEX = 255


# ------------------------------------------------------------
# Training
# ------------------------------------------------------------

SEED = 42

BATCH_SIZE = 8
NUM_POINTS = 16384

EPOCHS = 30

LEARNING_RATE = 1e-3
WEIGHT_DECAY = 1e-4

NUM_WORKERS = 4

CHECKPOINT_DIR = Path("ai/checkpoints")
BEST_MODEL_PATH = CHECKPOINT_DIR / "point_mlp_best.pt"