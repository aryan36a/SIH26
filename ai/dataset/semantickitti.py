from pathlib import Path

import numpy as np
import torch
from torch.utils.data import Dataset

from ai.config import (
    LEARNING_MAP,
    NUM_POINTS,
    IGNORE_INDEX,
)


class SemanticKITTIDataset(Dataset):
    """
    SemanticKITTI point-cloud dataset.

    Each sample returns:

        points:
            [N, 5]

            x
            y
            z
            intensity
            range

        labels:
            [N]

            semantic learning-class ID
    """

    def __init__(
        self,
        root,
        sequences,
        num_points=NUM_POINTS,
        training=True,
    ):
        self.root = Path(root)
        self.sequences = [str(s).zfill(2) for s in sequences]
        self.num_points = num_points
        self.training = training

        self.samples = []

        self._collect_samples()

        if not self.samples:
            raise RuntimeError(
                f"No SemanticKITTI samples found under: {self.root}"
            )

        print(
            f"Loaded {len(self.samples)} scans "
            f"from sequences {self.sequences}"
        )

    def _collect_samples(self):
        for sequence in self.sequences:

            velodyne_dir = (
                self.root
                / "sequences"
                / sequence
                / "velodyne"
            )

            labels_dir = (
                self.root
                / "sequences"
                / sequence
                / "labels"
            )

            if not velodyne_dir.exists():
                raise FileNotFoundError(
                    f"Missing Velodyne directory: {velodyne_dir}"
                )

            if not labels_dir.exists():
                raise FileNotFoundError(
                    f"Missing labels directory: {labels_dir}"
                )

            bin_files = sorted(velodyne_dir.glob("*.bin"))

            for bin_path in bin_files:

                label_path = (
                    labels_dir / f"{bin_path.stem}.label"
                )

                if not label_path.exists():
                    continue

                self.samples.append(
                    (bin_path, label_path)
                )

    @staticmethod
    def _load_points(path):

        data = np.fromfile(
            path,
            dtype=np.float32,
        )

        if data.size % 4 != 0:
            raise ValueError(
                f"Invalid SemanticKITTI point file: {path}"
            )

        points = data.reshape(-1, 4)

        xyz = points[:, :3]
        intensity = points[:, 3:4]

        ranges = np.linalg.norm(
            xyz,
            axis=1,
            keepdims=True,
        )

        features = np.concatenate(
            [
                xyz,
                intensity,
                ranges,
            ],
            axis=1,
        )

        return features

    @staticmethod
    def _load_labels(path):

        labels = np.fromfile(
            path,
            dtype=np.uint32,
        )

        # SemanticKITTI stores semantic ID in
        # the lower 16 bits.
        semantic_ids = labels & 0xFFFF

        mapped = np.full(
            semantic_ids.shape,
            IGNORE_INDEX,
            dtype=np.int64,
        )

        for raw_id, learning_id in LEARNING_MAP.items():

            mapped[
                semantic_ids == raw_id
            ] = learning_id

        return mapped

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, index):

        bin_path, label_path = self.samples[index]

        points = self._load_points(bin_path)
        labels = self._load_labels(label_path)

        if len(points) != len(labels):
            raise ValueError(
                f"Point/label mismatch in {bin_path}: "
                f"{len(points)} points vs "
                f"{len(labels)} labels"
            )

        valid = np.all(
            np.isfinite(points),
            axis=1,
        )

        points = points[valid]
        labels = labels[valid]

        if self.training:

            if len(points) >= self.num_points:

                indices = np.random.choice(
                    len(points),
                    self.num_points,
                    replace=False,
                )

            else:

                indices = np.random.choice(
                    len(points),
                    self.num_points,
                    replace=True,
                )

        else:

            if len(points) >= self.num_points:

                indices = np.linspace(
                    0,
                    len(points) - 1,
                    self.num_points,
                    dtype=np.int64,
                )

            else:

                indices = np.random.choice(
                    len(points),
                    self.num_points,
                    replace=True,
                )

        points = points[indices]
        labels = labels[indices]

        points = torch.from_numpy(
            points.astype(np.float32)
        )

        labels = torch.from_numpy(
            labels.astype(np.int64)
        )

        return points, labels