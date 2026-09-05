import random

import numpy as np
import torch
import torch.nn as nn
from torch.utils.data import DataLoader

from ai.config import (
    DATASET_ROOT,
    TRAIN_SEQUENCES,
    VAL_SEQUENCES,
    NUM_CLASSES,
    NUM_FEATURES,
    NUM_POINTS,
    BATCH_SIZE,
    EPOCHS,
    LEARNING_RATE,
    WEIGHT_DECAY,
    NUM_WORKERS,
    BEST_MODEL_PATH,
    CHECKPOINT_DIR,
    SEED,
    IGNORE_INDEX,
)

from ai.dataset.semantickitti import (
    SemanticKITTIDataset,
)

from ai.models.point_mlp import (
    PointMLP,
)

from ai.evaluation.metrics import (
    confusion_matrix,
    calculate_iou,
    calculate_accuracy,
)


def set_seed(seed):

    random.seed(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)

    if torch.cuda.is_available():
        torch.cuda.manual_seed_all(seed)


def normalize_features(points):
    """
    Normalize features using fixed engineering scales.

    This is the first baseline.
    Dataset-derived statistics can replace this later.
    """

    scale = points.new_tensor([
        50.0,    # x
        50.0,    # y
        10.0,    # z
        1.0,     # intensity
        100.0,   # range
    ])

    return points / scale


def train_one_epoch(
    model,
    loader,
    optimizer,
    criterion,
    device,
):

    model.train()

    total_loss = 0.0

    for points, labels in loader:

        points = points.to(device)
        labels = labels.to(device)

        points = normalize_features(points)

        optimizer.zero_grad()

        logits = model(points)

        loss = criterion(
            logits.reshape(-1, NUM_CLASSES),
            labels.reshape(-1),
        )

        loss.backward()

        optimizer.step()

        total_loss += loss.item()

    return total_loss / len(loader)


@torch.no_grad()
def evaluate(
    model,
    loader,
    criterion,
    device,
):

    model.eval()

    total_loss = 0.0

    matrix = torch.zeros(
        NUM_CLASSES,
        NUM_CLASSES,
        dtype=torch.long,
    )

    for points, labels in loader:

        points = points.to(device)
        labels = labels.to(device)

        points = normalize_features(points)

        logits = model(points)

        loss = criterion(
            logits.reshape(-1, NUM_CLASSES),
            labels.reshape(-1),
        )

        total_loss += loss.item()

        predictions = logits.argmax(
            dim=-1
        )

        matrix += confusion_matrix(
            predictions.cpu(),
            labels.cpu(),
            NUM_CLASSES,
            IGNORE_INDEX,
        )

    iou, mean_iou = calculate_iou(matrix)

    accuracy = calculate_accuracy(matrix)

    return (
        total_loss / len(loader),
        mean_iou,
        accuracy,
        iou,
    )


def main():

    set_seed(SEED)

    device = torch.device(
        "cuda"
        if torch.cuda.is_available()
        else "cpu"
    )

    print(f"Using device: {device}")

    train_dataset = SemanticKITTIDataset(
        DATASET_ROOT,
        TRAIN_SEQUENCES,
        NUM_POINTS,
        training=True,
    )

    val_dataset = SemanticKITTIDataset(
        DATASET_ROOT,
        VAL_SEQUENCES,
        NUM_POINTS,
        training=False,
    )

    train_loader = DataLoader(
        train_dataset,
        batch_size=BATCH_SIZE,
        shuffle=True,
        num_workers=NUM_WORKERS,
        pin_memory=torch.cuda.is_available(),
    )

    val_loader = DataLoader(
        val_dataset,
        batch_size=BATCH_SIZE,
        shuffle=False,
        num_workers=NUM_WORKERS,
        pin_memory=torch.cuda.is_available(),
    )

    model = PointMLP(
        num_classes=NUM_CLASSES,
        input_features=NUM_FEATURES,
    ).to(device)

    criterion = nn.CrossEntropyLoss(
        ignore_index=IGNORE_INDEX
    )

    optimizer = torch.optim.AdamW(
        model.parameters(),
        lr=LEARNING_RATE,
        weight_decay=WEIGHT_DECAY,
    )

    scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(
        optimizer,
        T_max=EPOCHS,
    )

    CHECKPOINT_DIR.mkdir(
        parents=True,
        exist_ok=True,
    )

    best_miou = -1.0

    for epoch in range(1, EPOCHS + 1):

        train_loss = train_one_epoch(
            model,
            train_loader,
            optimizer,
            criterion,
            device,
        )

        (
            val_loss,
            miou,
            accuracy,
            iou,
        ) = evaluate(
            model,
            val_loader,
            criterion,
            device,
        )

        scheduler.step()

        print(
            f"Epoch {epoch:03d}/{EPOCHS:03d} | "
            f"train loss {train_loss:.4f} | "
            f"val loss {val_loss:.4f} | "
            f"mIoU {miou:.4f} | "
            f"accuracy {accuracy:.4f}"
        )

        if miou > best_miou:

            best_miou = miou

            torch.save(
                {
                    "model_state_dict": model.state_dict(),
                    "num_classes": NUM_CLASSES,
                    "num_features": NUM_FEATURES,
                    "epoch": epoch,
                    "mIoU": miou,
                },
                BEST_MODEL_PATH,
            )

            print(
                f"Saved best model → "
                f"{BEST_MODEL_PATH}"
            )

    print()
    print(
        f"Training complete. "
        f"Best mIoU: {best_miou:.4f}"
    )


if __name__ == "__main__":
    main()