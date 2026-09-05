import torch


def confusion_matrix(
    predictions,
    targets,
    num_classes,
    ignore_index=255,
):
    """
    Build a [C, C] confusion matrix.

    Rows    = ground truth
    Columns = prediction
    """

    predictions = predictions.reshape(-1)
    targets = targets.reshape(-1)

    valid = targets != ignore_index

    predictions = predictions[valid]
    targets = targets[valid]

    valid = (
        (targets >= 0)
        & (targets < num_classes)
        & (predictions >= 0)
        & (predictions < num_classes)
    )

    predictions = predictions[valid]
    targets = targets[valid]

    indices = (
        targets * num_classes
        + predictions
    )

    matrix = torch.bincount(
        indices,
        minlength=num_classes * num_classes,
    )

    return matrix.reshape(
        num_classes,
        num_classes,
    )


def calculate_iou(matrix):

    intersection = torch.diag(matrix)

    ground_truth = matrix.sum(dim=1)

    predicted = matrix.sum(dim=0)

    union = (
        ground_truth
        + predicted
        - intersection
    )

    iou = intersection.float() / union.clamp_min(1).float()

    valid = union > 0

    if valid.any():
        mean_iou = iou[valid].mean().item()
    else:
        mean_iou = 0.0

    return iou, mean_iou


def calculate_accuracy(matrix):

    correct = torch.diag(matrix).sum()

    total = matrix.sum()

    if total == 0:
        return 0.0

    return (
        correct.float() / total.float()
    ).item()