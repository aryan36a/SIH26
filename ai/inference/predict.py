import argparse
from pathlib import Path

import numpy as np
import torch

from ai.config import (
    NUM_CLASSES,
    NUM_FEATURES,
    BEST_MODEL_PATH,
)

from ai.models.point_mlp import PointMLP


def load_point_cloud(path):

    data = np.fromfile(
        path,
        dtype=np.float32,
    )

    if data.size % 4 != 0:
        raise ValueError(
            "Invalid .bin file. "
            "Expected x,y,z,intensity."
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


def normalize_features(points):

    scale = np.array(
        [
            50.0,
            50.0,
            10.0,
            1.0,
            100.0,
        ],
        dtype=np.float32,
    )

    return points / scale


@torch.no_grad()
def predict(
    model,
    points,
    device,
):

    features = normalize_features(points)

    tensor = torch.from_numpy(
        features
    ).float().to(device)

    logits = model(
        tensor.unsqueeze(0)
    )

    probabilities = torch.softmax(
        logits,
        dim=-1,
    )

    confidence, class_ids = (
        probabilities.max(dim=-1)
    )

    return (
        class_ids.squeeze(0).cpu().numpy(),
        confidence.squeeze(0).cpu().numpy(),
    )


def main():

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--input",
        required=True,
        help="Path to SemanticKITTI .bin",
    )

    parser.add_argument(
        "--checkpoint",
        default=str(BEST_MODEL_PATH),
    )

    parser.add_argument(
        "--output",
        default="predictions.npz",
    )

    args = parser.parse_args()

    device = torch.device(
        "cuda"
        if torch.cuda.is_available()
        else "cpu"
    )

    checkpoint = torch.load(
        args.checkpoint,
        map_location=device,
    )

    model = PointMLP(
        num_classes=checkpoint["num_classes"],
        input_features=checkpoint["num_features"],
    )

    model.load_state_dict(
        checkpoint["model_state_dict"]
    )

    model.to(device)
    model.eval()

    points = load_point_cloud(
        Path(args.input)
    )

    class_ids, confidence = predict(
        model,
        points,
        device,
    )

    np.savez(
        args.output,
        class_ids=class_ids,
        confidence=confidence,
    )

    print(
        f"Processed {len(points)} points"
    )

    print(
        f"Predictions written to {args.output}"
    )


if __name__ == "__main__":
    main()